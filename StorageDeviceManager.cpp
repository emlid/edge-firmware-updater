#include "StorageDeviceManager.h"
#include "WindowsStorageDeviceManager.h"

StorageDeviceManager::StorageDeviceManager(QObject* parent)
    : QObject(parent) {

}

std::unique_ptr<StorageDeviceManager> StorageDeviceManager::instance()
{
   return std::unique_ptr<StorageDeviceManager>(new WindowsStorageDeviceManager());
}