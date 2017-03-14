import copy
import sys
import inspect
import os
import re
import random
import six.moves.urllib.parse as urlparse

from oslo.config import cfg
from oslo.config import types
from oslo.config import iniparser
from oslo.utils import strutils

from sds.openstack.common import excutils
from sds.openstack.common import importutils
from sds.openstack.common import log as logging

CONF = cfg.CONF
LOG = logging.getLogger(__name__)


class IniConfigWriter(iniparser.BaseParser):

    default_section = 'DEFAULT'
    backend_key_name = 'enabled_backends'

    # mode is either update or delete
    def __init__(self, read_file, write_file, sections, mode):
        super(IniConfigWriter, self).__init__()
        self.read_file = read_file
        self.write_file = write_file
        self.update_sections = sections
        self.cur_section = None
        self.write_fd = None
        self.mode = mode
        self.skip_write = False
        
    def write_key_value(self, key, value):
        if self.skip_write:
            return #don't write 
        self.write_fd.write("%s=" % (key))
        if not isinstance(value, list):
            value = [value]
        for i in range(0, len(value)):
            if i == 0:
                self.write_fd.write("%s\n" % (value[i]))
            else:
                self.write_fd.write("\t%s\n" % (value[i]))
        
    def process_key_value(self, key, value):
        if self.cur_section in self.update_sections:
            if key in self.update_sections[self.cur_section]:
                value = self.update_sections[self.cur_section].pop(key)
        self.write_key_value(key, value)
        return None, []

    def process_section(self, section):
        if self.skip_write:
            return
        if section in self.update_sections:
            entries = self.update_sections[section]
            for key in entries:
                self.write_key_value(key, entries[key])
        
    def parse_and_write(self):
        # open file for writing
        self.write_fd = open(self.write_file, 'w')

        # this results in writing all sections that are not part of self.update_section entries
        with open(self.read_file) as f:
            key = None
            value = []

            # this code is borrowed from iniparser.parse()
            for line in f:
                line = line.rstrip()
                if not line:
                    # Blank line, ends multi-line values
                    if key:
                        key, value = self.process_key_value(key, value)
                    self.write_fd.write("\n")
                    continue
                elif line.startswith((' ', '\t')):
                    # Continuation of previous assignment
                    if key is None:
                        self.error_unexpected_continuation(line)
                    else:
                        value.append(line.lstrip())
                    continue

                if key:
                    # Flush previous assignment, if any
                    key, value = self.process_key_value(key, value)

                if line.startswith('['):
                    # Section start
                    section = self._get_section(line)
                    self.process_section(self.cur_section)
                    self.update_sections.pop(self.cur_section, None)
                    self.cur_section = section
                    self.skip_write = False
                    # if mode is delete -> check if this section needs to be deleted
                    # delete only if it is not default section
                    if self.mode == 'delete' and self.cur_section in self.update_sections and \
                        self.cur_section != self.default_section:
                        self.skip_write = True
                    if not self.skip_write:
                        self.write_fd.write("%s\n" % (line))
                elif line.startswith(('#', ';')):
                    if not self.skip_write:
                        self.write_fd.write("%s\n" % (line))
                else:
                    key, value = self._split_key_value(line)
                    if not key:
                        return self.error_empty_key(line)

            if key:
                # Flush previous assignment, if any
                key, value = self.process_key_value(key, value)


        if self.mode != 'delete':
            self.skip_write = False
            # write the remaining sections
            for section in self.update_sections:
                # process only if there are entries
                if len(self.update_sections[section]) > 0:
                    self.write_fd.write("\n[%s]\n" % (section))
                    self.process_section(section)

        # close the file
        self.write_fd.close()
        self.write_fd = None
        

    """
        Two stage processing: 
            stage 1: identify if sections need update - this uses ConfigParser
            stage 2: write to a new ini file
        search_sections: dictionary of [section, {section entries}] pairs that need to be used for searching
        upd_sections: dictionary of [section, {section entries}] pairs that either need to be added or replaced
        mode is either 'update' or 'delete'
        
        Returns:
            upd_sections that is modified with right ini sections used for changing ini files
    """
    @classmethod
    def change_backends(cls, read_file, write_file, search_sections, upd_sections, mode):
        # local copy
        new_sections = dict(upd_sections)

        # checks if section entries have search keys and values 
        # returns True if all entries are present in a given section
        def _contains(cur_entries, search_entries):
            for key in search_entries:
                if key in cur_entries:
                    value = search_entries[key]
                    if not isinstance(value, list):
                        value = [value]
                    if sorted(value) != sorted(cur_entries[key]):
                        return False
                else:
                    return False
            return True

        # stage 1: parse file
        parser = cfg.ConfigParser(read_file, {})
        parser.parse()
        
        # search for matching section
        for s_section in search_sections:
            search_entries = search_sections[s_section] 
            for section in parser.sections:
                if _contains(parser.sections[section], search_entries):
                    # replace the s_section with section contents
                    new_sections[section] = new_sections.pop(s_section)
                    break
            
        # need to either create 'enabled_backends' or update the existing one
        backend_value = set()
        if cls.default_section in parser.sections:
            if cls.backend_key_name in parser.sections[cls.default_section]:
                value = parser.sections[cls.default_section][cls.backend_key_name]
                if isinstance(value, list):
                    if len(value) > 0:
                        backend_value = set(re.split(r'[,\s]\s*', value[0].strip()))
                    else:
                        value = set()
                else:
                    backend_value = set(re.split(r'[,\s]\s*', value.strip()))
        
        # add or remove the names in enabled_backends based on mode
        for section in new_sections:
            if (mode == 'update'):
                backend_value.add(section)
            else:
                backend_value.discard(section)
              
        # create new ini file with either new or merged section entries
        sections = {cls.default_section : {cls.backend_key_name : ','.join(backend_value)}}
        sections.update(new_sections)
        ini_write = cls(read_file, write_file, sections, mode)
        ini_write.parse_and_write()

        return new_sections
