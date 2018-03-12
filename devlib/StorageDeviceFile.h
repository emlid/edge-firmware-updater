#ifndef STORAGEDEVICEFILE_H
#define STORAGEDEVICEFILE_H

#include <QtCore>

namespace devlib {
    class IStorageDeviceFile;
}

class devlib::IStorageDeviceFile : public QFile
{
    Q_OBJECT
public:
    virtual ~IStorageDeviceFile(void) = default;

    virtual bool open(OpenMode mode) override final { return open_core(mode); }
    virtual void close(void) override final { close_core(); }
    virtual auto fileName() const
        -> QString override final { return fileName_core(); }

protected:
    virtual auto readData(char* data, qint64 len)
        -> qint64 override final { return readData_core(data, len);}

    virtual auto writeData(char const* data, qint64 len)
        -> qint64 override final { return writeData_core(data, len); }

    virtual auto seek(qint64 pos)
        -> bool override final { return seek_core(pos); }

private:
    virtual bool open_core(OpenMode mode) = 0;
    virtual void close_core(void) = 0;

    virtual auto readData_core(char* data, qint64 len) -> qint64 = 0;
    virtual auto writeData_core(char const* data, qint64 len) -> qint64 = 0;
    virtual auto fileName_core() const -> QString = 0;
    virtual auto seek_core(qint64) -> bool = 0;
};

#endif // STORAGEDEVICEFILE_H
