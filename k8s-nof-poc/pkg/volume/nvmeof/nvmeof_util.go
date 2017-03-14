/*
Copyright 2015 The Kubernetes Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

package nvmeof

import (
	"fmt"
	"os"
	"path"
	"strings"
    "syscall"

	"github.com/golang/glog"
	"k8s.io/kubernetes/pkg/util/mount"
	"k8s.io/kubernetes/pkg/volume"
)

// stat a path, if not exists, retry maxRetries times
// when nvmeof transports other than default are used,  use glob instead as pci id of device is unknown
type StatFunc func(string) (os.FileInfo, error)
type GlobFunc func(string) ([]string, error)

// make a directory like /var/lib/kubelet/plugins/kubernetes.io/nvmeof/portal-some_iqn-lun-lun_id
func makePDNameInternal(host volume.VolumeHost, traddr string, trsvcid int32, transport string, nqn string) string {
	return path.Join(host.GetPluginDir(nvmeofPluginName), fmt.Sprintf("%s-%d-%s-%s", traddr, trsvcid, transport, nqn))
}

type NVMEOFUtil struct{}

func (util *NVMEOFUtil) MakeGlobalPDName(nvmeof nvmeofDisk) string {
	return makePDNameInternal(nvmeof.plugin.host, nvmeof.traddr, nvmeof.trsvcid, nvmeof.transport, nvmeof.nqn)
}

func (util *NVMEOFUtil) getNvmeofDrives(b nvmeofDiskMounter) ([]string, error) {
    out, err :=  b.plugin.execCommand("nvme", []string{"list"})
    if (err != nil) { return nil, err }

    // get list of NVME drives that are not not local
    // NOTE: This is a hack!!
    var drvs []string
    lines := strings.Split(string(out), "\n")
    for _, ln := range(lines) {
        if strings.Contains(ln, " Linux   ") {
            if fields := strings.Fields(ln); len(fields) > 3 && fields[2] == "Linux" {
                if  drv := strings.Split(fields[0], "/"); len(drv) >= 3 {
                    drvs = append(drvs, drv[2])
                }
            }
        }
    }

    return drvs, nil
}

func (util *NVMEOFUtil) mountNVMeoF(b nvmeofDiskMounter) (error) {
    fd, err := syscall.Open("/dev/nvme-fabrics", syscall.O_RDWR, 0644)
    if (err != nil) { return err }

    var n int
    buf := fmt.Sprintf("traddr=%s,transport=%s,trsvcid=%d,nqn=%s", b.traddr, b.transport,
                        b.trsvcid, b.nqn)
    glog.V(10).Infof("NVMEOF: mountNVMeoFbuf=%s", buf)
    n, err = syscall.Write(fd, []byte(buf))
    if (err != nil) { return err }
    if (n != len(buf)) {
        return fmt.Errorf("syscall.Write size mismatch [%d, %d]", n, len(buf))
    }

    rbuf := make([]byte, 128, 1024)
    _, err = syscall.Read(fd, rbuf)
    if (err != nil) { return err }

    err = syscall.Close(fd)
    if (err != nil) { return err }

    glog.V(10).Infof("rbuf = %s", rbuf)
    return nil
}

func (util *NVMEOFUtil) connectNVMeoF(b nvmeofDiskMounter) (string, error) {
    oldDrvs, err := util.getNvmeofDrives(b)
    if err != nil { return "", err }

    err = util.mountNVMeoF(b)
    if err != nil { return "", err }

    newDrvs, err := util.getNvmeofDrives(b)
    if err != nil { return "", err }

    for _, drv := range(newDrvs) {
        found := false
        for _, old := range(oldDrvs) {
            if drv == old {
                found = true
                break
            }
        }
        if !found { return drv, nil }
    }
    return "", fmt.Errorf("Unable to locat NVMe-oF mounted drive")
}

func (util *NVMEOFUtil) disconnectNVMeoF(b nvmeofDiskUnmounter, path string) (error) {
    if  tokens := strings.Split(path, "/"); len(tokens) == 3 {
        drive := tokens[2]
        _, err :=  b.plugin.execCommand("nvme", []string{"disconnect", "-d", drive})
        return err
    } else {
        return fmt.Errorf("NVMEOF: disconnectNVMeoF unable to extract drive from %s", path)
    }
}

func (util *NVMEOFUtil) AttachDisk(b nvmeofDiskMounter) error {
    // connect to NVMe-oF target 
    // once this is done, you should see drive /dev/nvme<> 
    drv, err := util.connectNVMeoF(b)
    glog.V(10).Infof("util.connectNVMeoF returned drv: %s, err: %v", drv, err)
    if err != nil { return err }

	// mount it
	globalPDPath := b.manager.MakeGlobalPDName(*b.nvmeofDisk)
	notMnt, err := b.mounter.IsLikelyNotMountPoint(globalPDPath)
	if !notMnt {
		glog.V(10).Infof("nvmeof: %s already mounted", globalPDPath)
		return nil
	}

	if err := os.MkdirAll(globalPDPath, 0750); err != nil {
		glog.Errorf("nvmeof: failed to mkdir %s, error", globalPDPath)
		return err
	}

    devicePath := "/dev/" + drv
	err = b.mounter.FormatAndMount(devicePath, globalPDPath, b.fsType, nil)
	if err != nil {
		glog.Errorf("nvmeof: failed to mount nvmeof volume %s [%s] to %s, error %v", devicePath, b.fsType, globalPDPath, err)
	}

	return err
}

func (util *NVMEOFUtil) DetachDisk(c nvmeofDiskUnmounter, mntPath string) error {
	device, cnt, err := mount.GetDeviceNameFromMount(c.mounter, mntPath)
    glog.V(10).Infof("NVMEOF: DetachDisk mntPath:%s, device:%s", mntPath, device)
	if err != nil {
		glog.Errorf("nvmeof detach disk: failed to get device from mnt: %s\nError: %v", mntPath, err)
		return err
	}
	if err = c.mounter.Unmount(mntPath); err != nil {
		glog.Errorf("nvmeof detach disk: failed to unmount: %s\nError: %v", mntPath, err)
		return err
	}
	// if device is no longer used, see if need to logout the target
	if cnt <= 1 {
        err = util.disconnectNVMeoF(c, device) 
        if (err != nil) {
            return fmt.Errorf("nvmeof: failed to disconnect device %s:Error: %v", device, err)
        }
        glog.Infof("nvmeof: successfully disconnected device %s", device)
	}
	return nil
}
