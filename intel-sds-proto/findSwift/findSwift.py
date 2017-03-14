import json
import requests
import ipaddress
import sys
import swiftclient
import os
import logging
import argparse

# Setup Logging
parse = argparse.ArgumentParser()
parse.add_argument("--log", help="LOG=CRITICAL|ERROR|WARNING|INFO|DEBUG")
parse.add_argument("--debug", action='store_true', help="Enable debug flows")
parse.add_argument("--port", help="Swift listen port (default:8080)")
args = parse.parse_args()

# Get Port# where Swift should be listening
# For SS-Controller installations, it's Port 80, otherwise 8080
if not args.port:
    args.port = 8080
else:
    args.port=int(args.port)
    logging.debug("Listening on %d" % args.port)

if not args.log:
    args.log = 'INFO'
logging.basicConfig(level=args.log.upper(), format='%(asctime)s - %(levelname)s - %(message)s')

# Turn down the logging level of the requests module
requests_log = logging.getLogger("requests")
requests_log.setLevel(logging.WARNING)

class SwiftNode:
    def __init__(self, hostIP, user = 'test:tester', password='testing'):
        self.inPool         = False
        self.authKey        = 0
        self.authURL        = 0
        self.hostIP         = hostIP
        self.containerCount = 0
        self.bytesInUse     = 0
        self.user           = user
        self.password       = password
        self.containers     = []
        self.policies       = []
        self.fob            = "Object"

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'


""" Core routines start here:
    1) Scan Subnet looking for Swift Info
    2) Dump out the Swift Info"""
def main():
    # Get list of Swift Installations on Subnet
    swiftList = []
    scanSubnet(u'10.0.0.0/24', swiftList)
    if len(swiftList) == 0:
        logging.warning('No Swift Nodes found in the Subnet')

    # Get Auth Token for Each Installation
    getAuthKeys(swiftList)

    # Use Swift Client to get Container Info
    testSwiftClient(swiftList)
    print "Done Scanning!"

    # Dump Info found about SwiftNodes
    dumpSwiftNodes(swiftList)
    sys.exit(0)


def getAuthKeys(swiftList):
    # Grab Auth Key for each Swift Node in swiftList
    for node in swiftList:
       # url = 'http://' + node.hostIP.exploded + ':8080/auth/v1.0'
        url = 'http://' + node.hostIP.exploded + ':'+str(args.port)+'/auth/v1.0'
        logging.debug('Node User: %s, Node.pass: %s' % (node.user, node.password))
        headers = {'X-Storage-User' : node.user, 'X-Storage-Pass' : node.password}
        logging.debug('Headers are: %s', (headers))
        try:
            logging.debug("URL: %s, Headers: %s" % (url, headers))
            req = requests.get(url, headers=headers, timeout=1)
            logging.debug("Req: %s" % req)
            if (req.status_code != requests.codes.ok):
                req.raise_for_status()
            if req.status_code == 200:
                node.authKey = req.headers['x-auth-token']
                node.authURL = req.headers['x-storage-url']
                for key, value in req.headers.iteritems():
                   logging.debug('Key: %s, Value: %s', key, value)
        except Exception as e:
            logging.error('Failed on getAuthKeys for host: %s', node.hostIP)
            logging.error('Exception raised: %s' % e)
            node.containerCount = "Unauthorized"
            node.containers     = "Unauthorized"
            node.bytesInUse     = "Unauthorized"

# Given a SwiftList and Auth Keys, we can query the Swift nodes for containers, etc
# using the Swift Client library
def testSwiftClient(swiftList):
    for node in swiftList:
        url = node.authURL
        token = node.authKey
        # Skip nodes we couldn't authorize
        if ((url == 0) or (token == 0)):
            continue
        print ("Url: %s, Token: %s" % (url, token))
        info = swiftclient.client.head_account(url, token)
        # Get Number of Containers on Test Account
        for key, value in info.iteritems():
            logging.debug('Swift Client Key: %s, Value: %s', key, value)
        node.containerCount = info['x-account-container-count']
        node.bytesInUse = info['x-account-bytes-used']

        # Get Container Names
        info = swiftclient.client.get_account(url, token)
        # Skip the headers returned
        info = info[1:]
        for containers in info:
            for container in containers:
                node.containers.append(container['name'])
                for key, value in container.iteritems():
                    logging.debug('Container: Key: %s, Value: %s', key, value)

        # Get Storage Policy Names from Capabilities Call
#        url2 = 'http://' + node.hostIP.exploded + ':8080/info'
        url2 = 'http://' + node.hostIP.exploded + ':'+str(args.port)+'/info'
        conn_obj =  swiftclient.client.http_connection(url2)
        info = swiftclient.client.get_capabilities(conn_obj)
        node.policies = info['swift']['policies']
        for key, value in info.iteritems():
            logging.debug('Policies Key: %s, Value: %s', key, value)

def dumpSwiftNodes(swiftList):
    for node in swiftList:
        print('========================= SWIFT NODE ===========================')
        print("%-16s%-16s%-16s%-16s" % ('HostIP', 'User', '# Containers', 'ContainerNames'))
        print("%-16s%-16s%-16s%-16s" % (node.hostIP, node.user, node.containerCount, node.containers))
        print("%-16s" % ('BytesUsed'))
        print("%-16s" % ( node.bytesInUse))
        for policy in node.policies:
            default = ''
            # Do we have the 'default' key set for this policy?
            if 'default' in policy:
                if policy['default'] == True:
                    default = '(Default)'
            print "Policy: %s %s" % (policy['name'], default)
    print('================================================================')

def dumpSwiftInfo(req):
    if (req.status_code != requests.codes.ok):
        req.raise_for_status()
    print bcolors.OKGREEN + ("Found SWIFT Node at: %s " % (req.url)) + bcolors.ENDC
    # Grab SwiftInfo Dictionary
    swiftInfo = req.json()
    print bcolors.OKGREEN + "Version: " + swiftInfo['swift']['version'] + bcolors.ENDC

# Scan a subnet, dump any Swift info found and return the list of
# IP addresses that have a Swift instance running
def scanSubnet(network, swiftList):
    if args.debug:
    # DEBUG - uncomment these lines to scan specific nodes
        net = []  
        net.append(ipaddress.ip_address(u'10.0.0.51')) 
        net.append(ipaddress.ip_address(u'10.0.0.55')) 
        net.append(ipaddress.ip_address(u'10.0.0.11')) 
        net.append(ipaddress.ip_address(u'10.0.0.99')) 
        print net
    else:
        net = list(ipaddress.ip_network(network).hosts())

    logging.info('Scanning: %d Nodes on this subnet: %s',len(net), network)
    for host in net:
        #print host.exploded
#        url = 'http://' + host.exploded + ':8080/info'
        url = 'http://' + host.exploded + ':'+str(args.port)+'/info'
        try:
            dumpSwiftInfo(requests.get(url, timeout=.01))
            # Create a new SwiftNode instance and Append it to our list of Swift Installations
            # If we have a User/Key already defined, use it, else just rely on defaults
            if 'ST_USER' in os.environ and 'ST_KEY' in os.environ:
                newSwift = SwiftNode(host, os.environ['ST_USER'], os.environ['ST_KEY'])
            else:
                newSwift = SwiftNode(host)
            swiftList.append(newSwift)
            # Ignore all connection errors for now
        except (requests.exceptions.HTTPError, requests.exceptions.Timeout, requests.exceptions.ConnectionError):
            pass
            #print("INFO: No Swift at URL: %s" %(url))

    return swiftList

if __name__ == "__main__":
    main()

# Test code here which is not executed at this point
"""
url='http://10.0.0.51:8080/auth/v1.0'
user='test:tester'
password='testing'
headers = {'X-Storage-User' : 'test:tester', 'X-Storage-Pass' : 'testing'}
#curl -v -H 'X-Storage-User: test:tester' -H 'X-Storage-Pass: testing' http://10.0.0.51:8080/auth/v1.0

#req = requests.get(url)
# Working headers - get the auth-token and URL
req = requests.get(url, headers=headers)
if (req.status_code != 200):
    print("INFO: No Swift at URL: %s" %(url))
    exit(-INFONOSWIFT)

    #Where discovery runs - avoid the proxy lookup by putting all IP addresses in 'no_proxy' zone
#printf -v no_proxy '%s,' 10.0.0.{1..254}
#export no_proxy="${no_proxy%,}";

#url='http://10.0.0.51:8080/auth/v1.0'
#user='test:tester'
#password='testing'
#headers = {'X-Storage-User' : 'test:tester', 'X-Storage-Pass' : 'testing'}

Test Code End """

