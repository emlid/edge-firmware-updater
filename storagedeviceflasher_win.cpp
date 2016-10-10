#include "storagedeviceflasher_win.h"
#include <QFileInfo>
#include <QTime>
#include <QDebug>
#include <handledevice_win.h>

StorageDeviceFlasher::StorageDeviceFlasher(QObject *parent):
    QObject(parent)
{

}

int StorageDeviceFlasher::flashDevice(struct FlashingParameters params)
{
    bool passfail = true;
    QString fileName = params.inputFile;
    QString drive = params.outputFile;

        QFileInfo fileinfo(fileName);
        if (fileinfo.exists() && fileinfo.isFile() &&
                fileinfo.isReadable() && (fileinfo.size() > 0) )
        {
            st.bytesSent = 0;
            st.fileSize = fileinfo.size();

            if (fileName.at(0) == drive.at(0))
            {
                flasherLog("Image file cannot be located on the target device.", true);
                return 1;
            }

            st.flashingStatus = STATUS_WRITING;

            unsigned long long i, availablesectors, numsectors;

            int volumeID = drive.at(0).toLatin1() - 'A';
            int deviceID;
            getPhysicalDriveNumber(drive, &deviceID);

            HANDLE hVolume;
            hVolume = getHandleOnVolume(volumeID, GENERIC_WRITE);
            if (hVolume == INVALID_HANDLE_VALUE)
            {
                st.flashingStatus = STATUS_IDLE;
                return 1;
            }

            if (!getLockOnVolume(hVolume))
            {
                CloseHandle(hVolume);
                st.flashingStatus = STATUS_IDLE;
                hVolume = INVALID_HANDLE_VALUE;
                return 1;
            }
            if (!unmountVolume(hVolume))
            {
                removeLockOnVolume(hVolume);
                CloseHandle(hVolume);
                st.flashingStatus = STATUS_IDLE;
                hVolume = INVALID_HANDLE_VALUE;
                return 1;
            }

            const wchar_t * encodedName = reinterpret_cast<const wchar_t *>(fileName.utf16());

            HANDLE hFile;
            hFile = getHandleOnFile(encodedName, GENERIC_READ);

            if (hFile == INVALID_HANDLE_VALUE)
            {
                removeLockOnVolume(hVolume);
                CloseHandle(hVolume);
                st.flashingStatus = STATUS_IDLE;
                hVolume = INVALID_HANDLE_VALUE;
                return 1;
            }

            HANDLE hRawDisk;
            hRawDisk = getHandleOnDevice(deviceID, GENERIC_WRITE);

            if (hRawDisk == INVALID_HANDLE_VALUE)
            {
                removeLockOnVolume(hVolume);
                CloseHandle(hFile);
                CloseHandle(hVolume);
                st.flashingStatus = STATUS_IDLE;
                hVolume = INVALID_HANDLE_VALUE;
                hFile = INVALID_HANDLE_VALUE;
                return 1;
            }
            availablesectors = getNumberOfSectors(hRawDisk, &sectorsize);
            numsectors = getFileSizeInSectors(hFile, sectorsize);

            if (numsectors > availablesectors)
            {
                flasherLog(QString("Not enough space on disk: Size: %1 sectors  Available: %2 sectors  Sector size: %3").arg(numsectors).arg(availablesectors).arg(sectorsize), true);

                removeLockOnVolume(hVolume);
                CloseHandle(hRawDisk);
                CloseHandle(hFile);
                CloseHandle(hVolume);
                st.flashingStatus = STATUS_IDLE;
                hVolume = INVALID_HANDLE_VALUE;
                hFile = INVALID_HANDLE_VALUE;
                hRawDisk = INVALID_HANDLE_VALUE;
                return 1;
            }

            QTime timer;
            timer.start();
            int percent = 0;
            char *sectorData;

            emit startFlashing();

            for (i = 0ul; i < numsectors && st.flashingStatus == STATUS_WRITING; i += 1024ul)
            {
                sectorData = readSectorDataFromHandle(hFile, i, (numsectors - i >= 1024ul) ? 1024ul:(numsectors - i), sectorsize);
                if (sectorData == NULL)
                {
                    delete sectorData;
                    removeLockOnVolume(hVolume);
                    CloseHandle(hRawDisk);
                    CloseHandle(hFile);
                    CloseHandle(hVolume);
                    st.flashingStatus = STATUS_IDLE;
                    sectorData = NULL;
                    hRawDisk = INVALID_HANDLE_VALUE;
                    hFile = INVALID_HANDLE_VALUE;
                    hVolume = INVALID_HANDLE_VALUE;

                    return 1;
                }
                if (!writeSectorDataToHandle(hRawDisk, sectorData, i, (numsectors - i >= 1024ul) ? 1024ul:(numsectors - i), sectorsize))
                {
                    delete sectorData;
                    removeLockOnVolume(hVolume);
                    CloseHandle(hRawDisk);
                    CloseHandle(hFile);
                    CloseHandle(hVolume);
                    st.flashingStatus = STATUS_IDLE;
                    sectorData = NULL;
                    hRawDisk = INVALID_HANDLE_VALUE;
                    hFile = INVALID_HANDLE_VALUE;
                    hVolume = INVALID_HANDLE_VALUE;

                    return 1;
                }
                delete sectorData;
                sectorData = NULL;
                emit updateProgress(st.bytesSent, st.fileSize);
                current_summary(numsectors, i, &percent);
            }

            flasherLog(QString("%1 bytes (%2 MB) transferred in %3 s (%4  MB/s)").arg(st.bytesSent).arg(st.bytesSent / 1000000).arg(timer.elapsed()/1000).arg((double)(st.bytesSent / 1000000) / (timer.elapsed() / 1000)));

            removeLockOnVolume(hVolume);
            CloseHandle(hRawDisk);
            CloseHandle(hFile);
            CloseHandle(hVolume);
            sectorData = NULL;
            hRawDisk = INVALID_HANDLE_VALUE;
            hFile = INVALID_HANDLE_VALUE;
            hVolume = INVALID_HANDLE_VALUE;

            if (st.flashingStatus == STATUS_CANCELED){
                passfail = false;
            }
        }
        else if (!fileinfo.exists() || !fileinfo.isFile())
        {
            flasherLog("File error. The selected file does not exist.", true);
            passfail = false;
        }
        else if (!fileinfo.isReadable())
        {
            flasherLog("File error. You do not have permision to read the selected file.", true);
            passfail = false;
        }
        else if (fileinfo.size() == 0)
        {
            flasherLog("File Error. The specified file contains no data.", true);
            passfail = false;
        }

        if (passfail){
            flasherLog("Write Successful.");
        }


    st.flashingStatus = STATUS_IDLE;
    return 0;
}


void StorageDeviceFlasher::terminate(bool cancel) {
    if (!cancel) {
        /*
         *  functionality is not implemented yet
         */
    }
    st.flashingStatus = STATUS_CANCELED;
}


HANDLE StorageDeviceFlasher::getHandleOnFile(LPCWSTR filelocation, DWORD access)
{
    HANDLE hFile;
    hFile = CreateFileW((wchar_t *)filelocation, access, FILE_SHARE_READ, NULL, (access == GENERIC_READ) ? OPEN_EXISTING:CREATE_ALWAYS, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        wchar_t *errormessage=NULL;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0,
                         (LPWSTR)&errormessage, 0, NULL);
        QString errText = QString::fromUtf16((const ushort *)errormessage);

        flasherLog(QString("An error occurred when attempting to get a handle on the file. Error %1: %2").arg(GetLastError()).arg(errText), true);

        LocalFree(errormessage);
    }
    return hFile;
}

HANDLE StorageDeviceFlasher::getHandleOnDevice(int device, DWORD access)
{
    HANDLE hDevice;
    wchar_t devicename[] = L"\\\\.\\PhysicalDrive0";
    devicename[17] += device;

    hDevice = CreateFile( devicename, access, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        wchar_t *errormessage=NULL;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        flasherLog(QString("An error occurred when attempting to get a handle on the device. Error %1: %2").arg(GetLastError()).arg(errText), true);

        LocalFree(errormessage);
    }
    return hDevice;
}

HANDLE StorageDeviceFlasher::getHandleOnVolume(int volume, DWORD access)
{
    HANDLE hVolume;
    wchar_t volumename[] = L"\\\\.\\A:";
    volumename[4] += volume;

    hVolume = CreateFile(volumename, access, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (hVolume == INVALID_HANDLE_VALUE)
    {
        wchar_t *errormessage=NULL;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), LANG_SYSTEM_DEFAULT, (LPWSTR)&errormessage, 0, NULL);
        QString errText = QString::fromUtf16((const ushort *)errormessage);

        flasherLog(QString("An error occurred when attempting to get a handle on the volume. Error %1: %2").arg(GetLastError()).arg(errText), true);

        LocalFree(errormessage);
    }
    return hVolume;
}

bool StorageDeviceFlasher::getLockOnVolume(HANDLE handle)
{
    DWORD bytesreturned;
    BOOL bResult;
    bResult = DeviceIoControl(handle, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytesreturned, NULL);
    if (!bResult)
    {
        wchar_t *errormessage=NULL;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        flasherLog(QString("An error occurred when attempting to lock the volume. Error %1: %2").arg(GetLastError()).arg(errText), true);

       LocalFree(errormessage);
    }
    return (bResult);
}

bool StorageDeviceFlasher::removeLockOnVolume(HANDLE handle)
{
    DWORD junk;
    BOOL bResult;
    bResult = DeviceIoControl(handle, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &junk, NULL);
    if (!bResult)
    {
        wchar_t *errormessage=NULL;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        flasherLog(QString("An error occurred when attempting to unlock the volume.Error %1: %2").arg(GetLastError()).arg(errText), true);

        LocalFree(errormessage);
    }
    return (bResult);
}

bool StorageDeviceFlasher::unmountVolume(HANDLE handle)
{
    DWORD junk;
    BOOL bResult;
    bResult = DeviceIoControl(handle, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &junk, NULL);
    if (!bResult)
    {
        wchar_t *errormessage=NULL;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        flasherLog(QString("An error occurred when attempting to dismount the volume. Error %1: %2").arg(GetLastError()).arg(errText), true);

        LocalFree(errormessage);
    }
    return (bResult);
}

unsigned long long StorageDeviceFlasher::getNumberOfSectors(HANDLE handle, unsigned long long *sectorsize)
{
    DWORD junk;
    DISK_GEOMETRY_EX diskgeometry;
    BOOL bResult;
    bResult = DeviceIoControl(handle, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &diskgeometry, sizeof(diskgeometry), &junk, NULL);
    if (!bResult)
    {
        wchar_t *errormessage=NULL;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        flasherLog(QString("An error occurred when attempting to get the device's geometry.Error %1: %2").arg(GetLastError()).arg(errText));

        LocalFree(errormessage);
        return 0;
    }
    if (sectorsize != NULL)
    {
        *sectorsize = (unsigned long long)diskgeometry.Geometry.BytesPerSector;
    }
    return (unsigned long long)diskgeometry.DiskSize.QuadPart / (unsigned long long)diskgeometry.Geometry.BytesPerSector;
}

unsigned long long StorageDeviceFlasher::getFileSizeInSectors(HANDLE handle, unsigned long long sectorsize)
{
    unsigned long long retVal = 0;
    LARGE_INTEGER filesize;
    if(GetFileSizeEx(handle, &filesize) == 0)
    {
        wchar_t *errormessage=NULL;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        flasherLog(QString("An error occurred while getting the file size.Error %1: %2").arg(GetLastError()).arg(errText), true);

        LocalFree(errormessage);
        retVal = 0;
    }
    else
    {
        retVal = ((unsigned long long)filesize.QuadPart / sectorsize ) + (((unsigned long long)filesize.QuadPart % sectorsize )?1:0);
    }
    return(retVal);
}

bool StorageDeviceFlasher::spaceAvailable(char *location, unsigned long long spaceneeded)
{
    ULARGE_INTEGER freespace;
    BOOL bResult;
    bResult = GetDiskFreeSpaceEx((LPCWSTR)location, NULL, NULL, &freespace);
    if (!bResult)
    {
        wchar_t *errormessage=NULL;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        flasherLog(QString("Failed to get the free space on drive %1.Error %2: %3").arg(location).arg(GetLastError()).arg(errText), true);
        return true;
    }
    return (spaceneeded <= freespace.QuadPart);
}

char* StorageDeviceFlasher::readSectorDataFromHandle(HANDLE handle, unsigned long long startsector, unsigned long long numsectors, unsigned long long sectorsize)
{
    unsigned long bytesread;
    char *data = new char[sectorsize * numsectors];
    LARGE_INTEGER li;
    li.QuadPart = startsector * sectorsize;
    SetFilePointer(handle, li.LowPart, &li.HighPart, FILE_BEGIN);
    if (!ReadFile(handle, data, sectorsize * numsectors, &bytesread, NULL))
    {
        wchar_t *errormessage=NULL;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        flasherLog(QString("An error occurred when attempting to read data from handle.Error %1: %2").arg(GetLastError()).arg(errText), true);

        LocalFree(errormessage);
        delete data;
        data = NULL;
    }
    if (bytesread < (sectorsize * numsectors))
    {
            memset(data + bytesread,0,(sectorsize * numsectors) - bytesread);
    }
    return data;
}

bool StorageDeviceFlasher::writeSectorDataToHandle(HANDLE handle, char *data, unsigned long long startsector, unsigned long long numsectors, unsigned long long sectorsize)
{
    unsigned long byteswritten;
    BOOL bResult;
    LARGE_INTEGER li;
    li.QuadPart = startsector * sectorsize;
    SetFilePointer(handle, li.LowPart, &li.HighPart, FILE_BEGIN);
    bResult = WriteFile(handle, data, sectorsize * numsectors, &byteswritten, NULL);
    if (!bResult)
    {
        wchar_t *errormessage = NULL;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        flasherLog(QString("An error occurred when attempting to write data to handle.\nError %1: %2").arg(GetLastError()).arg(errText), true);

        LocalFree(errormessage);
    }
    st.bytesSent += sectorsize * numsectors;
    return (bResult);
}


void StorageDeviceFlasher::current_summary(unsigned long long numsectors, int i, int *lastValue) {
    long int percent = (numsectors % 100) == 0 ? numsectors/100 : numsectors/100+1;
    if ( i / (percent * 1) > (*lastValue)){
        qDebug() << "\r [" <<i / percent<< "%] "<<i <<" blocks ("<<i*sectorsize/1000000<< "MB) written.";
        (*lastValue)++;
    }
}


bool StorageDeviceFlasher::getPhysicalDriveNumber(QString drivename, int *pid)
{
    QString expandName = "\\\\.\\" + drivename + '\0';
    wchar_t name[expandName.size()];
    expandName.toWCharArray(name);

    HANDLE hDevice;
    PSTORAGE_DEVICE_DESCRIPTOR pDevDesc;
    DEVICE_NUMBER deviceInfo;

    wchar_t *nameWithSlash;
    wchar_t *nameNoSlash;

    // some calls require no tailing slash, some require a trailing slash...
    if ( !(slashify(name, &nameWithSlash, &nameNoSlash)) )
    {
        return 0;
    }
        hDevice = CreateFile(nameNoSlash, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDevice == INVALID_HANDLE_VALUE)
        {
            wchar_t *errormessage = NULL;
            FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
            QString errText = QString::fromUtf16((const ushort *)errormessage);
            flasherLog(QString("An error occurred when attempting to get a handle." "Error %1: %2").arg(GetLastError()).arg(errText), true);
            LocalFree(errormessage);

            return 0;
        }
        else
        {
            int arrSz = sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1;
            pDevDesc = (PSTORAGE_DEVICE_DESCRIPTOR)new BYTE[arrSz];
            pDevDesc->Size = arrSz;

            GetDisksProperty(hDevice, pDevDesc, &deviceInfo);
            *pid = deviceInfo.DeviceNumber;

            delete pDevDesc;
            CloseHandle(hDevice);
        }


    // free the strings allocated by slashify
    free(nameWithSlash);
    free(nameNoSlash);

    return 1;
}
