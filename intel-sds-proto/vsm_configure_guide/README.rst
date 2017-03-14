About vsm_configure_guide
==================================================================
vsm_configure_guide folder is used to describe how to configure VSM
environment with OpenStack.
VSM(Virtual Storage Manager) is an open-source software that provides
storage cluster management based on Ceph. For the installation or any
further questions, please refer to link:
https://github.com/01org/virtual-storage-manager

Get start
==================================================================
Notice: 
Before started, please ensure that your openstack environment is ready
as well as VSM controller node.

0) copy this folder to the openstack node
1) Configure the params in file deploy.ini
2) Run `install.sh` script:
       ./install.sh
3) After `install.sh` complete, it will generate folder `vsm_controller_node`
4) Copy folder `vsm_controller_node` to the VSM controller node.
5) Login VSM controller node and enter `vsm_controller_node` folder and run
   `vsm_controller_config.sh` script:
       ./vsm_controller_config.sh <openstack_IP> <admin_pass> <agent_pass> <vsm_service_pass>

To enable cephx authentication copy content of your /etc/ceph/ from your vsm storage nodes to openstack node.