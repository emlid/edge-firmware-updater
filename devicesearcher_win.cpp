#include <initguid.h>
#include <windows.h>
#include <setupapi.h>
#include <ntddstor.h>

#include "firmwareupgradecontroller.h"
#include "rpiboot.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "libusb-1.0/libusb.h"
#include <handledevice_win.h>
#include <QDebug>

void DeviceSearcher::startFindBoardLoop()
{
    emit searcherMessage("Scan for devices...");

    int bootable = findBootableDevices();
    qDebug() << "bootable:" << bootable;

    if (bootable > 0) {

        QThread rpibootThread;
        emit searcherMessage(QString("Found %1 bootable device%2").arg(bootable).arg(bootable > 1 ? "s":""));

        emit searcherMessage("Rpiboot started");
        startRpiBoot(&rpibootThread, bootable);
        rpibootThread.wait();
        emit searcherMessage("Rpiboot finished");
    }

    //get a pointer to the object that sent the signal for start searching
    FirmwareUpgradeController *controller = (FirmwareUpgradeController *)QObject::sender();

    if (!(controller->recoveryModeEnabled)) {
        enumerateDevices();
    } else {
        enumerateRawDevices();
    }

    emit searcherMessage("Search finished");
    emit searchFinished();
}

int DeviceSearcher::findBootableDevices()
{
    libusb_context *context = 0;
    libusb_device **list = 0;
    int ret = 0;
    ssize_t count = 0;
    int bootable = 0;

    ret = libusb_init(&context);
    Q_ASSERT(ret == 0);

    count = libusb_get_device_list(context, &list);
    Q_ASSERT(count > 0);

    for (ssize_t idx = 0; idx < count; ++idx) {
        libusb_device *device = list[idx];
        struct libusb_device_descriptor desc;

        ret = libusb_get_device_descriptor(device, &desc);
        Q_ASSERT(ret == 0);

        // FIXME: remove hardcode from conditions
        if (desc.idVendor == 2652 && (desc.idProduct == 10083 || desc.idProduct == 10084)) {
           bootable++;
        }
    }

    libusb_exit(context);
    return bootable;
}

void DeviceSearcher::enumerateDevices()
{
        // GetLogicalDrives returns 0 on failure, or a bitmask representing
        // the drives available on the system (bit 0 = A:, bit 1 = B:, etc)
        unsigned long driveMask = GetLogicalDrives();
        int i = 0;

        int removableDisks = 0;

        while (driveMask != 0)
        {
            if (driveMask & 1)
            {
                wchar_t drivename[] = L"\\\\.\\A:\\";
                drivename[4] += i;

                if (checkDriveType(drivename))
                {
                    qDebug() << (QString::fromWCharArray(&drivename[4]));
                    emit foundDevice(0, 0, QString::fromWCharArray(&drivename[4]));
                    removableDisks++;
                }
            }
            driveMask >>= 1;
            ++i;
        }
        if (removableDisks == 0) {
            emit searcherMessage("No removable device found", true);
        } else {
            emit searcherMessage(QString("Found %1 storage device%2").arg(removableDisks).arg(removableDisks > 1 ? "s":""));
        }
}

int DeviceSearcher::enumerateRawDevices()
{
    HDEVINFO diskClassDevices;
    GUID diskClassDeviceInterfaceGuid = GUID_DEVINTERFACE_DISK;
    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData;
    DWORD requiredSize;
    DWORD deviceIndex;

    HANDLE disk = INVALID_HANDLE_VALUE;
    STORAGE_DEVICE_NUMBER diskNumber;
    DWORD bytesReturned;

    /*
     * Get the handle to the device information set for installed
     * disk class devices. Returns only devices that are currently
     * present in the system and have an enabled disk device
     * interface.
     */
    diskClassDevices = SetupDiGetClassDevs(&diskClassDeviceInterfaceGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (diskClassDevices == INVALID_HANDLE_VALUE) {
        emit searcherMessage("SetupDiGetClassDevs failed", true);
        return GetLastError();
    }

    ZeroMemory( &deviceInterfaceData, sizeof( SP_DEVICE_INTERFACE_DATA ) );
    deviceInterfaceData.cbSize = sizeof( SP_DEVICE_INTERFACE_DATA );
    deviceIndex = 0;
    int removableDisks = 0;

    while (SetupDiEnumDeviceInterfaces( diskClassDevices, NULL, &diskClassDeviceInterfaceGuid, deviceIndex, &deviceInterfaceData)) {

        ++deviceIndex;

        SetupDiGetDeviceInterfaceDetail(diskClassDevices, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);
        if (GetLastError() == INVALID_HANDLE_VALUE) {
            emit searcherMessage("SetupDiGetDeviceInterfaceDetail failed", true);
            return GetLastError();
        }

        deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(requiredSize);

        if (deviceInterfaceDetailData == NULL) {
            emit searcherMessage("Memory allocation failed", true);
            return GetLastError();
        }

        ZeroMemory( deviceInterfaceDetailData, requiredSize );
        deviceInterfaceDetailData->cbSize = sizeof( SP_DEVICE_INTERFACE_DETAIL_DATA );


        bool ret;
        ret = SetupDiGetDeviceInterfaceDetail(diskClassDevices, &deviceInterfaceData, deviceInterfaceDetailData, \
                                              requiredSize, NULL, NULL);
        if (!ret) {
            emit searcherMessage("SetupDiGetDeviceInterfaceDetail failed", true);
            return GetLastError();
        }

        disk = CreateFile(deviceInterfaceDetailData->DevicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, \
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (disk == INVALID_HANDLE_VALUE) {
            emit searcherMessage("CreateFile failed", true);
            return GetLastError();
        }

        ret = DeviceIoControl(disk, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &diskNumber, sizeof(STORAGE_DEVICE_NUMBER), \
                              &bytesReturned, NULL);
        if (!ret) {
            emit searcherMessage("DeviceIoControl failed", true);
            return GetLastError();
        }

        CloseHandle(disk);
        disk = INVALID_HANDLE_VALUE;

        QString volumes = mapDeviceWithRemovableVolumes(diskNumber.DeviceNumber);

        if (!volumes.isEmpty()){
            emit foundDevice(0, 0, QString("\\\\?\\PhysicalDrive%1 [%2] ").arg(diskNumber.DeviceNumber).arg(volumes));
            removableDisks++;
        }

        free(deviceInterfaceDetailData);
    }

    if (diskClassDevices != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(diskClassDevices);
    }

    if (disk != INVALID_HANDLE_VALUE) {
        CloseHandle(disk);
    }

    if (removableDisks == 0) {
        emit searcherMessage("No removable physical drives found", true);
    } else {
        emit searcherMessage(QString("Found %1 removable physical drive%2").arg(removableDisks).arg(removableDisks > 1 ? "s":""));
    }

    return 0;
}

QString DeviceSearcher::mapDeviceWithRemovableVolumes(uint deviceNumber)
{
    unsigned long driveMask = GetLogicalDrives();
    int i = 0;
    QString volumesOnRemovableDevice = "";

    while (driveMask != 0)
    {
        if (driveMask & 1)
        {
            char drivename = 'A';
            drivename += i;
            uint pid = 0;

            QString expandName = QString("\\\\.\\%1:").arg(drivename);
            wchar_t name[expandName.size()];
            expandName.toWCharArray(name);

            HANDLE hDevice;
            PSTORAGE_DEVICE_DESCRIPTOR pDevDesc;
            DEVICE_NUMBER deviceInfo;

            hDevice = CreateFile(name, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
            if (hDevice == INVALID_HANDLE_VALUE)
            {
                wchar_t *errormessage = NULL;
                FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
                QString errText = QString::fromUtf16((const ushort *)errormessage);
                searcherMessage(QString("An error occurred when attempting to get a handle." "Error %1: %2").arg(GetLastError()).arg(errText), true);
                LocalFree(errormessage);

                return "";
            } else {
                int arrSz = sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1;
                pDevDesc = (PSTORAGE_DEVICE_DESCRIPTOR)new BYTE[arrSz];
                pDevDesc->Size = arrSz;

                GetDisksProperty(hDevice, pDevDesc, &deviceInfo);
                pid = deviceInfo.DeviceNumber;

                delete pDevDesc;
                CloseHandle(hDevice);
            }

            if (pid == deviceNumber) {
                if (checkDriveType(name))
                {
                    if (volumesOnRemovableDevice.isEmpty()) {
                        volumesOnRemovableDevice.append(QString("%1:\\").arg(drivename));
                    } else {
                        volumesOnRemovableDevice.append(QString(", %1:\\").arg(drivename));
                    }
                }
            }
        }
        driveMask >>= 1;
        ++i;
    }

    return volumesOnRemovableDevice;
}
