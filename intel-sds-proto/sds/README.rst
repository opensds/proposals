Introduction - Software Defined Storage (SDS) Controller Prototype
==================================================================
SDS prototype includes three packages - 1) SDS controller 2) Python sdsclient and
3) SDS Horizon plug-in. This package provides SDS controller services that can be 
consumed by both python sds client, sds horizon plug-in and in future 3rd party 
clients. Controller prototype includes functionality for provisioning 
(only addition of storage capacity), automated discovery of storage systems and 
composition of virtual pools. Provisioning works only with Ceph Virtual Storage
Manager (Ceph VSM) provisioning software. Both discovery and virtual pool composition 
works only with Ceph, Swift storage systems. Other storage systems can be integrated 
using the specific drivers. Intent of this prototype effort is to understand how to
evolve APIs as a generic framework for delivering integrated storage management 
functions by taking advantage of existing OpenStack services. For example, pool
creation automatically configures Cinder volume types and updates cinder.conf file 
with right pool information that can be consumed by Cinder services. 

Prerequisites
=============
The SDS Controller Prototype is intended to be used in an OpenStack
environment.  It relies on Keystone services for authentication, MySQL
for database services, RabbitMQ for messaging and Horizon dashboard for controller
plug-in.  Install OpenStack components (base services, Cinder, Horizon etc.) using
OpenStack install guide: http://docs.openstack.org/juno/install-guide/install/apt/content/.

The SDS Controller Prototype was developed and tested on an Ubuntu 14.04 Server.  
For the "Discovery" routines, the Swift and Ceph clusters were running in separate 
VMs, accessible within the same subnet.

Installation Steps
==================
These install steps assume all in one install for OpensStack and SDS Controller 
services.

1. Install, Configure and Test Juno OpenStack Services:
Use sections 1-9 in the OpenStack install guide for Juno:
http://docs.openstack.org/juno/install-guide/install/apt/content/
to install basic environment as well as :
- MySQL
- RabbitMQ
- Keystone services.
To install, test, and develop with the SDS Controller Prototype, other
services, such as Cinder and Horizon, are required.

2. Install VSM, Configure and Test 3 node Ceph Storage Cluster provisioning:
Use https://01.org/virtual-storage-manager for instructions on how to install,
setup and configure Ceph clusters. NOTE: VSM runs on CentOS 6.5. Install VSM client on  
Openstack controller node (Ubuntu 14.04) and rest of the components on separate nodes. 
Ensure VSM is configured to use OpenStack keystone service installed in previous step.

3. Install and Configure SDS prototype source code from git repo or unpack the tar file provided to you:
It is preferable to install SDS controller on OpenStack Controller Node.

3.a: Installing from Git repo:
$ git clone <git repo>
$ cd sds-prototype
$ cd sds
$ sudo python setup.py build
$ sudo python setup.py install

Installing from tar ball:
$ tar -xzf <sds compressed tar>
$ cd sds-prototype
$ cd sds
$ sudo python setup.py build
$ sudo python setup.py install

3.b: Create a user named sds and change password
$ sudo useradd -s /bin/bash -d /home/sds -m sds
$ sudo passwd sds

3.c: Create sds directories:
$ sudo mkdir -p /etc/sds
$ sudo mkdir -p /var/lib/sds
$ sudo mkdir -p /var/run/sds
$ sudo mkdir -p /var/log/sds

3.d: Copy sds config files to /etc/sds directory:
$ cd sds-prototype/sds/etc/sds
$ sudo cp -pr * /etc/sds/
$ sudo cp /etc/sds/sds.conf.sample /etc/sds/sds.conf
$ sudo cp /etc/sds/logging_sample.conf /etc/sds/logging.conf

3.e: Copy sds services startup scripts (Note: This is for Ubuntu only):
$ cd sds-prototype/sds/scripts
$ sudo cp sds-api.conf /etc/init/

3.f: Change owner to sds user and sds group for all sds directories:
$ sudo chown sds:sds -R /etc/sds
$ sudo chown sds:sds -R /var/lib/sds
$ sudo chown sds:sds -R /var/run/sds
$ sudo chown sds:sds -R /var/log/sds
$ sudo chown sds:sds -R /etc/init/sds-api.conf

3.h: Create sds database (NOTE: replace <SDS_PASSWORD> with password you created for sds):
$ sudo mysql -u root -p 
sql> create database sds;
sql> grant all privileges on sds.* to sds@"localhost" identified by "<SDS_PASSWORD>";
sql> grant all privileges on sds.* to sds@"%" identified by "<SDS_PASSWORD>";
sql> quit;

3.i: Create keystone service credentials for sds:
Export admin tenant variables 
$ export OS_TENANT_NAME=<admin tenant name>
$ export OS_USERNAME=<user name>
$ export OS_PASSWORD=ADMIN_PASS
$ export OS_AUTH_URL=http://<OPENSTACK CONTROLLER NODE>:35357/v2.0

Create sds keystone user - replace SDS_CONTROLLER_PASS with right controller password
$ keystone user-create --name sds --pass <SDS_CONTROLLER_PASS> --enabled true

Add the admin role to the sds user
$ keystone user-role-add --user sds --tenant service --role admin

Create the sds service and API end points
$ keystone service-create --name sds --type storage_controller_v1 \
  --description "Open SDS Storage Controller"
$ keystone endpoint-create \
    --region RegionOne \
    --service_id $(keystone service-list | awk '/ storage_controller_v1 / {print $2}')\
    --publicurl "http://<OPENSTACK CONTROLLER NODE>:9776/v1/%(tenant_id)s"  \
    --adminurl "http://<OPENSTACK CONTROLLER NODE>:9776/v1/%(tenant_id)s"  \
    --internalurl "http://<OPENSTACK CONTROLLER NODE>:9776/v1/%(tenant_id)s"

3.j: Edit /etc/sds/sds.conf with right credentials and configuration:
In the [database] section, configure database access. Replace SDS_PASSWORD with the 
password you chose for the sds database. Replace <OPENSTACK CONTROLLER NODE> with the controller
node or IP address.

[database]
connection = mysql://sds:<SDS_PASSWORD>@<OPENSTACK CONTROLLER NODE>sds

In the [DEFAULT] section, configure RabbitMQ message broker access:
Replace RABBIT_PASS with the password you chose for the guest account in RabbitMQ.
Replace <OPENSTACK CONTROLLER NODE> with the controller node or IP address.

[DEFAULT]
rpc_backend = sds.openstack.common.rpc.impl_kombu
rabbit_host = <OPENSTACK CONTROLLER NODE>
rabbit_password = RABBIT_PASS

In the [DEFAULT] and [keystone_authtoken] sections, configure Identity service access:
onfigure cinder connectivity - needed to create cinder volume types and get existing
volume type extra-specs:
[cinder]
api_version=2
auth_uri = http://<OPENSTACK CONTROLLER NODE>:5000/v2.0/
admin_tenant_name=service
admin_user=cinder
admin_password=<CINDER_PASSWD>
os_user=cinder

Configure ceph instance information - needed to push this to cinder.conf when a pool is
created:
[cinder_ceph]
rbd_secret_uuid=<uuid>

Configure swift instance information - needed for auto discovery (optional). If this is not used,
it will use keystone for discovering swift service:
[swift]
#this can be keystone, tempauth or template
#v1.0 - http://<SWIFT PROXY NODE>/auth/v1.0
#v1.0 template - http%secure%://%host%%port%/auth/v1.0
#v2.0 - http://<OPENSTACK CONTROLLER NODE>:5000/v2.0/
#auth_uri=http://<OPENSTACK CONTROLLER NODE>:5000/v2.0/
#auth_version=2.0
#user=swift
#key=<SWIFT KEY>
#tenant_name=<ADMIN TENANT NAME>


[DEFAULT]
auth_strategy = keystone
 
[keystone_authtoken]
auth_uri = http://<OPENSTACK CONTROLLER NODE>:5000/v2.0
auth_host = <OPENSTACK CONTROLLER NODE>
auth_port = 35357
auth_protocol = http
admin_tenant_name = service
admin_user = sds
admin_password = <SDS_PASSWD>

To assist with troubleshooting, enable verbose logging in the [DEFAULT] section:
[DEFAULT]
verbose = True
debug = True

Configure cinder connectivity - needed to create cinder volume types and get existing
volume type extra-specs:
[cinder]
api_version=2
auth_uri = http://<OPENSTACK CONTROLLER NODE>:5000/v2.0/
admin_tenant_name=service
admin_user=cinder
admin_password=<CINDER_PASSWD>
os_user=cinder

Configure ceph instance information - needed to push this to cinder.conf when a pool is 
created:
[cinder_ceph]
rbd_secret_uuid=<uuid>

Configure swift instance information - needed for auto discovery (optional). If this is not used, 
it will use keystone for discovering swift service:
[swift]
#this can be keystone, tempauth or template
#v1.0 - http://<SWIFT PROXY NODE>/auth/v1.0
#v1.0 template - http%secure%://%host%%port%/auth/v1.0
#v2.0 - http://<OPENSTACK CONTROLLER NODE>:5000/v2.0/
#auth_uri=http://<OPENSTACK CONTROLLER NODE>:5000/v2.0/
#auth_version=2.0
#user=swift
#key=<SWIFT KEY>
#tenant_name=<ADMIN TENANT NAME>

3.k: Populate the SDS Storage database:
$ sudo sds-manage db sync

3.l: Start sds-api service:
sudo service sds-api start

4. Install the sdsclient command-line interface.  Refer to python sds-client install instructions:

5. Verify SDS Controller Prototype is Operational via sdsclient
$ sdsclient --version
2015.1

Verify the sdsclient can communicate with the SDS API:
$ sdsclient backend-list
+----+------+-----------------+---------------------+
| id | Name | config_specs_id | capability_specs_id |
+----+------+-----------------+---------------------+
+----+------+-----------------+---------------------+

6. Setup needed for creating pools automatically and configuring cinder.conf:
a. Enable login for both sds and cinder
$ sudo usermod -s /bin/bash sds
$ sudo usermod -s /bin/bash cinder

b. Log into sds and cinder accounts, generate ssh key and copy key to cinder nodes:
$ su - cinder
$ ssh-keygen
$ exit
$ su - sds
$ ssh-keygen

copy this to all cinder-volume nodes:
$ ssh-copy-id cinder@<cinder volume host> 
$ exit
