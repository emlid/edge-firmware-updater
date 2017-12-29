#ifndef FLASHERDATAOWNER_H
#define FLASHERDATAOWNER_H

#include <QtCore>
#include <memory>

#include "devlib/devlib.h"

class FlasherDataOwner {
public:
    FlasherDataOwner(void)
        : _image(nullptr), _device(nullptr)
    { }

    FlasherDataOwner(QString const& fileName,
                devlib::StorageDeviceInfo const& deviceInfo)
        : _image(std::make_shared<QFile>(fileName)),
          _device(std::make_shared<devlib::StorageDeviceFile>(deviceInfo))
    { }

    ~FlasherDataOwner() {
        Q_ASSERT_X(_device.unique(), "~FlasherDataOwner()", "device not unique");
        Q_ASSERT_X(_image.unique(),  "~FlasherDataOwner()", "image not unique");
    }

    std::shared_ptr<QFile> image(void) {
        Q_ASSERT(_image);
        return _image;
    }

    std::shared_ptr<devlib::StorageDeviceFile> device(void) {
        Q_ASSERT(_device);
        return _device;
    }

    bool reset(QString const& filename,
               devlib::StorageDeviceInfo const& info)
    {
        if (_image.unique() || _device.unique()) {
            qWarning() << "Trying to reset FlasherDataOwner with not unique ptrs";
            return false;
        }

        _image.reset(new QFile(filename));
        _device.reset(new devlib::StorageDeviceFile(info));

        return true;
    }

private:
    std::shared_ptr<QFile>                     _image;
    std::shared_ptr<devlib::StorageDeviceFile> _device;
};


#endif // FLASHERDATAOWNER_H
