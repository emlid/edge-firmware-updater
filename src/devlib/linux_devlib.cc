#include "linux_devlib.h"

using namespace devlib;

impl::StorageDeviceInfo_Private::
    StorageDeviceInfo_Private(int vid, int pid, const QString &devFilePath)
    : _vid(vid), pid(pid), _deviceFilePath(devFilePath)
{  }
