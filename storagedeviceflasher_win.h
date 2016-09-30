#ifndef STORAGEDEVICEFLASHER_H
#define STORAGEDEVICEFLASHER_H

#include <windows.h>
#include <QObject>


enum Status {
    STATUS_IDLE=0,
    STATUS_READING,
    STATUS_WRITING,
    STATUS_EXIT,
    STATUS_CANCELED
};

struct FlashingParameters{
    QString blockSize;
    QString inputFile;
    QString outputFile;

    FlashingParameters(): blockSize("1000000"), inputFile(""), outputFile("") {}
};

class StorageDeviceFlasher : public QObject
{
    Q_OBJECT

public:
    explicit StorageDeviceFlasher(QObject *parent = 0);
    int flashDevice(FlashingParameters);
    uint64_t getImageSize(void) {return st_size;}
    uint64_t getBytesSent(void) {return st_bytes;}
    void terminate(bool cancel = 0);

signals:
    void updateProgress(uint32_t bytesSent, uint32_t fileSize);
    void flasherMessage(const QString& text, bool critical);
    void startFlashing();

private slots:
    void flasherLog(const QString& text, bool critical = 0) {emit flasherMessage(text, critical);}

private:
    HANDLE getHandleOnFile(LPCWSTR filelocation, DWORD access);
    HANDLE getHandleOnDevice(int device, DWORD access);
    HANDLE getHandleOnVolume(int volume, DWORD access);
    bool getLockOnVolume(HANDLE handle);
    bool removeLockOnVolume(HANDLE handle);
    bool unmountVolume(HANDLE handle);
    unsigned long long getNumberOfSectors(HANDLE handle, unsigned long long *sectorsize);
    unsigned long long getFileSizeInSectors(HANDLE handle, unsigned long long sectorsize);
    bool spaceAvailable(char *location, unsigned long long spaceneeded);
    char *readSectorDataFromHandle(HANDLE handle, unsigned long long startsector, unsigned long long numsectors, unsigned long long sectorsize);
    bool writeSectorDataToHandle(HANDLE handle, char *data, unsigned long long startsector, unsigned long long numsectors, unsigned long long sectorsize);
    void current_summary(unsigned long long numsectors, int i, int *lastValue);
    bool getPhysicalDriveNumber(QString drivename, int *pid);


    uint64_t st_size;
    uint64_t st_bytes;
    unsigned long long sectorsize;
};

#endif // STORAGEDEVICEFLASHER_H
