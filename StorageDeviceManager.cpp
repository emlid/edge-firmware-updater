#include "StorageDeviceManager.h"

#if defined(Q_OS_WIN)
#include "windows/WindowsStorageDeviceManager.h"
#elif defined(Q_OS_LINUX)
#include "linux/LinuxStorageDeviceManager.h"
#endif


StorageDeviceManager::StorageDeviceManager(QObject* parent)
    : QObject(parent)
{ }


std::unique_ptr<StorageDeviceManager> StorageDeviceManager::instance()
{
#if defined(Q_OS_WIN)
    return std::unique_ptr<StorageDeviceManager>(new WindowsStorageDeviceManager());
#else
    return std::unique_ptr<StorageDeviceManager>(new LinuxStorageDeviceManager());
#endif
}
