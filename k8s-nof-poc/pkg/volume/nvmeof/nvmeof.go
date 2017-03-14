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

    "github.com/golang/glog"
    "k8s.io/kubernetes/pkg/api/v1"
    "k8s.io/kubernetes/pkg/types"
    "k8s.io/kubernetes/pkg/util/exec"
    "k8s.io/kubernetes/pkg/util/mount"
    utilstrings "k8s.io/kubernetes/pkg/util/strings"
    "k8s.io/kubernetes/pkg/volume"
    ioutil "k8s.io/kubernetes/pkg/volume/util"
)

// This is the primary entrypoint for volume plugins.
func ProbeVolumePlugins() []volume.VolumePlugin {
    return []volume.VolumePlugin{&nvmeofPlugin{nil, exec.New()}}
}

type nvmeofPlugin struct {
    host volume.VolumeHost
    exe  exec.Interface
}

var _ volume.VolumePlugin = &nvmeofPlugin{}
var _ volume.PersistentVolumePlugin = &nvmeofPlugin{}

const (
    nvmeofPluginName = "kubernetes.io/nvmeof"
)

func (plugin *nvmeofPlugin) Init(host volume.VolumeHost) error {
    plugin.host = host
    return nil
}

func (plugin *nvmeofPlugin)  GetPluginName() string {
    return nvmeofPluginName
}

func (plugin *nvmeofPlugin) GetVolumeName(spec *volume.Spec) (string, error) {
    nvmeof, _, err := plugin.getVolumeSource(spec)
    if err != nil {
        return "", err
    }

    return fmt.Sprintf("%v:%v:%v:%v",
                        nvmeof.Transport,
                        nvmeof.Nqn,
                        nvmeof.TrAddr,
                        nvmeof.TrSvcId), nil 
}

func (plugin *nvmeofPlugin) CanSupport(spec *volume.Spec) bool {
    if (spec.Volume != nil && spec.Volume.NVMEOF == nil) || 
       (spec.PersistentVolume != nil && spec.PersistentVolume.Spec.NVMEOF == nil) {
        return false
    }

    return true
}

func (plugin *nvmeofPlugin) RequiresRemount() bool {
    return false
}

func (plugin *nvmeofPlugin) GetAccessModes() []v1.PersistentVolumeAccessMode {
    return []v1.PersistentVolumeAccessMode{
        v1.ReadWriteOnce,
        v1.ReadOnlyMany,
    }
}

func (plugin *nvmeofPlugin) NewMounter(spec *volume.Spec, pod *v1.Pod, _ volume.VolumeOptions) (volume.Mounter, error) {
    return plugin.newMounterInternal(spec, pod.UID, &NVMEOFUtil{}, plugin.host.GetMounter())
}

func (plugin *nvmeofPlugin) newMounterInternal(spec *volume.Spec, podUID types.UID, manager diskManager, mounter mount.Interface) (volume.Mounter, error) {
    nvmeof, readOnly, err := plugin.getVolumeSource(spec)
    if (err != nil) {
        return nil, err
    }

    return &nvmeofDiskMounter{
        nvmeofDisk: &nvmeofDisk{
            volName:   spec.Name(),
            podUID:    podUID,
            transport: nvmeof.Transport,
            nqn:       nvmeof.Nqn,
            traddr:    nvmeof.TrAddr,
            trsvcid:   nvmeof.TrSvcId,
            manager:   manager,
            plugin:    plugin},
        fsType:     nvmeof.FSType,
        readOnly:   readOnly,
        mounter:    &mount.SafeFormatAndMount{Interface: mounter, Runner: exec.New()},
        deviceUtil: ioutil.NewDeviceHandler(ioutil.NewIOHandler()),
    }, nil

}

func (plugin *nvmeofPlugin) NewUnmounter(volName string, podUID types.UID) (volume.Unmounter, error) {
    return plugin.newUnmounterInternal(volName, podUID, &NVMEOFUtil{}, plugin.host.GetMounter())
}

func (plugin *nvmeofPlugin) newUnmounterInternal(volName string, podUID types.UID, manager diskManager, mounter mount.Interface) (volume.Unmounter, error) {
    return &nvmeofDiskUnmounter{
        nvmeofDisk: &nvmeofDisk{
            podUID:  podUID,
            volName: volName,
            manager: manager,
            plugin:  plugin,
        },
        mounter: mounter,
    }, nil
}

func (plugin *nvmeofPlugin) execCommand(command string, args []string) ([]byte, error) {
    cmd := plugin.exe.Command(command, args...)
    return cmd.CombinedOutput()
}

func (plugin *nvmeofPlugin)  ConstructVolumeSpec(volumeName, mountPath string) (*volume.Spec, error) {
    mounter := plugin.host.GetMounter()
    pluginDir := plugin.host.GetPluginDir(plugin.GetPluginName())
    sourceName, err := mounter.GetDeviceNameFromMount(mountPath, pluginDir)
    if err != nil {
        return nil, err
    }
    glog.V(10).Infof("Found volume %s mounted to %s", sourceName, mountPath)

    nvmeofVolume := &v1.Volume{
        Name: volumeName,
        VolumeSource: v1.VolumeSource{
            NVMEOF: &v1.NVMEOFVolumeSource{
                Nqn: volumeName,
            },
        },
    }
    return volume.NewSpecFromVolume(nvmeofVolume), nil
}

func (plugin *nvmeofPlugin) getVolumeSource(spec *volume.Spec) (*v1.NVMEOFVolumeSource, bool, error) {
    if spec.Volume != nil && spec.Volume.NVMEOF != nil {
        return spec.Volume.NVMEOF, spec.Volume.NVMEOF.ReadOnly, nil
    } else if spec.PersistentVolume != nil &&
        spec.PersistentVolume.Spec.NVMEOF != nil {
        return spec.PersistentVolume.Spec.NVMEOF, spec.ReadOnly, nil
    }

    return nil, false, fmt.Errorf("Spec does not reference an NVMEOF volume type")
}

type nvmeofDisk struct {
    volName    string
    podUID     types.UID
    transport  string
    nqn        string
    traddr     string
    trsvcid    int32
    plugin     *nvmeofPlugin
    // Utility interface that provides API calls to the provider to attach/detach disks.
    manager    diskManager
    volume.MetricsNil
}

func (nvmeof *nvmeofDisk) GetPath() string {
    name := nvmeofPluginName
    // safe to use PodVolumeDir now: volume teardown occurs before pod is cleaned up
    return nvmeof.plugin.host.GetPodVolumeDir(nvmeof.podUID, utilstrings.EscapeQualifiedNameForDisk(name), nvmeof.volName)
}

type nvmeofDiskMounter struct {
    *nvmeofDisk
    readOnly   bool
    fsType     string
    mounter    *mount.SafeFormatAndMount
    deviceUtil ioutil.DeviceUtil
}

var _ volume.Mounter = &nvmeofDiskMounter{}

func (b *nvmeofDiskMounter) GetAttributes() volume.Attributes {
    return volume.Attributes{
        ReadOnly:        b.readOnly,
        Managed:         !b.readOnly,
        SupportsSELinux: true,
    }
}

// Checks prior to mount operations to verify that the required components (binaries, etc.)
// to mount the volume are available on the underlying node.
// If not, it returns an error
func (b *nvmeofDiskMounter) CanMount() error {
    return nil
}

func (b *nvmeofDiskMounter) SetUp(fsGroup *int64) error {
    return b.SetUpAt(b.GetPath(), fsGroup)
}

func (b *nvmeofDiskMounter) SetUpAt(dir string, fsGroup *int64) error {
    // diskSetUp checks mountpoints and prevent repeated calls
    err := diskSetUp(b.manager, *b, dir, b.mounter, fsGroup)
    if err != nil {
        glog.Errorf("nvmeof: failed to setup mount %s %v", dir, err)
    }
    return err
}

type nvmeofDiskUnmounter struct {
    *nvmeofDisk
    mounter mount.Interface
}

var _ volume.Unmounter = &nvmeofDiskUnmounter{}

// Unmounts the bind mount, and detaches the disk only if the disk
// resource was the last reference to that disk on the kubelet.
func (c *nvmeofDiskUnmounter) TearDown() error {
    return c.TearDownAt(c.GetPath())
}

func (c *nvmeofDiskUnmounter) TearDownAt(dir string) error {
    return diskTearDown(c.manager, *c, dir, c.mounter)
}
