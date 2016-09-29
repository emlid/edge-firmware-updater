#ifndef HANDLEDEVICE_WIN_H
#define HANDLEDEVICE_WIN_H

#include <QString>
#include <windows.h>

typedef struct _DEVICE_NUMBER
{
    DEVICE_TYPE  DeviceType;
    ULONG  DeviceNumber;
    ULONG  PartitionNumber;
} DEVICE_NUMBER, *PDEVICE_NUMBER;


bool slashify(wchar_t *, wchar_t **, wchar_t **);
bool checkDriveType(wchar_t *name);
BOOL GetDisksProperty(HANDLE , PSTORAGE_DEVICE_DESCRIPTOR , DEVICE_NUMBER *);

#endif // HANDLEDEVICE_WIN_H
