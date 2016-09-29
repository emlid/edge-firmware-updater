#ifndef STORAGEDEVICEFLASHER_H
#define STORAGEDEVICEFLASHER_H

#include <QObject>

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
    uint64_t st_size;
    uint64_t st_bytes;
};

#endif // STORAGEDEVICEFLASHER_H
