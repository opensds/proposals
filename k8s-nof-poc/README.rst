Introduction
------------

nvmeof_poc branch contains Kubernetes driver for NVMe over Fabrics Kernel Module.

NVMe over Fabrics is in experimental stage. You can use Ubuntu 16.10 or custom kernel
build to get NVMe over fabrics kernel modules. You will need two servers with RDMA
capable NICs to test NVMe over Fabrics driver. Below steps show setip Chelsio iWARP 
NIC cards.

Development Environment Setup
-----------------------------

1. On NVMe over Fabrics node, install Kubernetes development packages. Refer to
Kubernetes developer documentation on how to setup development environment: 
https://github.com/kubernetes/community/tree/master/contributors/devel


2. Install NVMe command line tools on NVMe initiator (aka K8S dev node) and
NVMe target node

a. Install nvme command line tool on both nodes
    apt install nvme-cli

b. On target node, install nvmetcli command line tool
    Install confgshell:
        git clone https://github.com/open-iscsi/configshell-fb.git

        cd configshell-fb

        python setup.py install

    Install nvmetcli tool:
        wget http://git.infradead.org/users/hch/nvmetcli.git/snapshot/f4cba8c61605599dc84f81fbb27b314532975e65.tar.gz

        cd /root/nvmetcli-be4d196

        python setup.py install


c. Load the kernel modules
    On initiator and target, load nic drivers:

    modprobe iw_cxgb4

    modprobe rdma_ucm    

    On target, run the following commands:

    modprobe null_blk

    modprobe nvmet

    modprobe nvmet-rdma 

    On initiator, run the following commands:

    modprobe nvme

    modprobe nvme-rdma
        
d. On target node, expose local drives as NVMe over Fabrics target drives:
    cd <nvmet cli directory>

    Edit rdma.json with the right drive, ip address and protocol

        {
          "hosts": [
            {
              "nqn": "hostnqn"
            }
          ],
          "ports": [
            {
              "addr": {
                "adrfam": "ipv4",
                "traddr": "<ip address here>",
                "treq": "not specified",
                "trsvcid": "4420",
                "trtype": "rdma"
              },
              "portid": 2,
              "referrals": [],
              "subsystems": [
                "testnqn"
              ]
            }
          ],
          "subsystems": [
            {
              "allowed_hosts": [],
              "attr": {
                "allow_any_host": "1"
              },
              "namespaces": [
                {
                  "device": {
                    "nguid": "ef90689c-6c46-d44c-89c1-4067801309a8",
                    "path": "/dev/nvme0n1"
                  },
                  "enable": 1,
                  "nsid": 1
                }
              ],
              "nqn": "testnqn"
            }
          ]
        }


    ./nvmetcli restore rdma.json

    ./nvmetcli 

    > ls


e. On initiator node, discover the targets

    nvme discover -t rdma -a <ip address> -s 4420

3. On NVMe initiator node, start up Kubernetes local single node cluster

    cd <kubernetes directory>

    hack/local-up-cluster.sh

4. On NVMe initiator node, open a different terminal window to launch pod. 

    <k8s dir>/example folder has nvmf.yaml script. Make changes to point to right target ip address

    export KUBERNETES_PROVIDER=local

    cluster/kubectl.sh create -f nvmf.yaml

    cluster/kubectl.sh get pods

    lsblk
        NAME    MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
        nvme1n1 259:1    0   1.5T  0 disk /var/lib/kubelet/pods/a07237b1-db7f-11e6-93f5-000743341f10/volumes/kubernetes.io~nvmeof/nvmev1

    cluster/kubectl.sh delete pod <pod name>

