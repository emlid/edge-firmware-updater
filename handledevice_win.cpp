#include <handledevice_win.h>
#include <rpiboot.h>
#include <flashing_parameters.h>
#include "libusb-1.0/libusb.h"
#include <Windows.h>
#include <wchar.h>
#include <iostream>
#include <QString>
#include <QTextStream>
#include <QDebug>
using namespace std;

BOOL GetDisksProperty(HANDLE hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc,
                      DEVICE_NUMBER *devInfo)
{
    STORAGE_PROPERTY_QUERY Query; // input param for query
    DWORD dwOutBytes; // IOCTL output length
    BOOL bResult; // IOCTL return val
    BOOL retVal = true;
    DWORD cbBytesReturned;

    // specify the query type
    Query.PropertyId = StorageDeviceProperty;
    Query.QueryType = PropertyStandardQuery;

    // Query using IOCTL_STORAGE_QUERY_PROPERTY
    bResult = ::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
                &Query, sizeof(STORAGE_PROPERTY_QUERY), pDevDesc,
                pDevDesc->Size, &dwOutBytes, (LPOVERLAPPED)NULL);
    if (bResult)
    {
        bResult = ::DeviceIoControl(hDevice, IOCTL_STORAGE_GET_DEVICE_NUMBER/*IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS*/ ,
                    NULL, 0, devInfo, sizeof(DEVICE_NUMBER), &dwOutBytes,
                    (LPOVERLAPPED)NULL);
        if (!bResult)
        {
            retVal = false;
            wchar_t *errormessage = NULL;
            FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
            QString errText = QString::fromUtf16((const ushort *)errormessage);
            qWarning()<<QObject::tr("An error occurred while getting the device number.\n" "This usually means something is currently accessing the device; please close all applications and try again.\n\nError %1: %2").arg(GetLastError()).arg(errText);

            LocalFree(errormessage);
        }
    }
    else
    {
        if (DeviceIoControl(hDevice, IOCTL_STORAGE_CHECK_VERIFY2, NULL, 0, NULL, 0, &cbBytesReturned,
                            (LPOVERLAPPED) NULL))
        {
            wchar_t *errormessage=NULL;
            FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
            QString errText = QString::fromUtf16((const ushort *)errormessage);
            qWarning()<<QObject::tr("An error occurred while getting the device number.\n" "This usually means something is currently accessing the device; please close all applications and try again.\n\nError %1: %2").arg(GetLastError()).arg(errText);

            LocalFree(errormessage);
        }
            retVal = false;
    }

    return(retVal);
}

bool slashify(wchar_t *str, wchar_t **slash, wchar_t **noSlash)
{
    bool retVal = false;
    int strLen = wcslen(str);
    if ( strLen > 0 )
    {
        if ( *(str + strLen - 1) == '\\' )
        {
            // trailing slash exists
            if (( (*slash = (wchar_t *)calloc( (strLen + 1), sizeof(wchar_t))) != NULL) &&
                    ( (*noSlash = (wchar_t *)calloc(strLen, sizeof(wchar_t))) != NULL))
            {
                wcsncpy(*slash, str, strLen);
                wcsncpy(*noSlash, *slash, (strLen - 1));
                retVal = true;
            }
        }
        else
        {
            // no trailing slash exists
            if ( ((*slash = (wchar_t *)calloc( (strLen + 2), sizeof(wchar_t))) != NULL) &&
                 ((*noSlash = (wchar_t *)calloc( (strLen + 1), sizeof(wchar_t))) != NULL) )
            {
                wcsncpy(*noSlash, str, strLen);
                wprintf(*slash, "%s\\", *noSlash);
                retVal = true;
            }
        }
    }
    return(retVal);
}



bool checkDriveType(wchar_t *name)
{
    HANDLE hDevice;
    PSTORAGE_DEVICE_DESCRIPTOR pDevDesc;
    bool retVal = false;
    int driveType;
    DWORD cbBytesReturned;
    wchar_t *nameWithSlash;
    wchar_t *nameNoSlash;

    // some calls require no tailing slash, some require a trailing slash...
    if ( !(slashify(name, &nameWithSlash, &nameNoSlash)) )
    {
        return(retVal);
    }

    driveType = GetDriveType(name);
    switch( driveType )
    {
    case DRIVE_REMOVABLE: // The media can be removed from the drive.
    case DRIVE_FIXED:     // The media cannot be removed from the drive.
        hDevice = CreateFile(nameNoSlash, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDevice == INVALID_HANDLE_VALUE)
        {
            wchar_t *errormessage = NULL;
            FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
            QString errText = QString::fromUtf16((const ushort *)errormessage);
            qWarning()<<QObject::tr("An error occurred when attempting to get a handle on .\n" "Error %1: %2").arg(GetLastError()).arg(errText);
            QString debugname = QString::fromUtf16((const ushort *)name);
            qWarning() << debugname;
            LocalFree(errormessage);
        }
        else
        {
            int arrSz = sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1;
            pDevDesc = (PSTORAGE_DEVICE_DESCRIPTOR)new BYTE[arrSz];
            pDevDesc->Size = arrSz;

            // check if the drive is
            // removable or (fixed AND on the usb bus, SD, or MMC (undefined in XP/mingw))

            if(( ((driveType == DRIVE_REMOVABLE))
                      || ( (driveType == DRIVE_FIXED) && ((pDevDesc->BusType == BusTypeUsb) || (pDevDesc->BusType == 0xC) || (pDevDesc->BusType == 0xD)) ) ) )
            {
                // ensure that the drive is actually accessible
                // multi-card hubs were reporting "removable" even when empty

                if(DeviceIoControl(hDevice, IOCTL_STORAGE_CHECK_VERIFY2, NULL, 0, NULL, 0, &cbBytesReturned, (LPOVERLAPPED) NULL))
                {
                    retVal = true;
                }
                else
                    // IOCTL_STORAGE_CHECK_VERIFY2 fails on some devices under XP/Vista, try the other (slower) method, just in case.
                {
                    CloseHandle(hDevice);
                    hDevice = CreateFile(nameNoSlash, FILE_READ_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
                    if(DeviceIoControl(hDevice, IOCTL_STORAGE_CHECK_VERIFY, NULL, 0, NULL, 0, &cbBytesReturned, (LPOVERLAPPED) NULL))
                    {
                        retVal = true;
                    }
                }
            }

            delete pDevDesc;
            CloseHandle(hDevice);
        }

        break;
    default:
        retVal = false;
    }

    // free the strings allocated by slashify
    free(nameWithSlash);
    free(nameNoSlash);

    return(retVal);
}


