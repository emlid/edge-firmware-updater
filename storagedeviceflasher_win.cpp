#include "storagedeviceflasher_win.h"

StorageDeviceFlasher::StorageDeviceFlasher(QObject *parent):
    QObject(parent)
{

}

int StorageDeviceFlasher::flashDevice(struct FlashingParameters params) {
    /*
     *  functionality is not implemented yet
     */
    return 0;
}


void StorageDeviceFlasher::terminate(bool cancel) {
    if (!cancel) {
        /*
         *  functionality is not implemented yet
         */
    }
}
