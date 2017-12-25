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
        Q_ASSERT(_device.use_count() <= 1);
        Q_ASSERT(_image.use_count() <= 1);
    }

    std::shared_ptr<QFile> image(void) const {
        Q_ASSERT(_image != nullptr);
        return _image;
    }

    std::shared_ptr<devlib::StorageDeviceFile> device(void) const {
        Q_ASSERT(_device != nullptr);
        return _device;
    }

    bool reset(QString const& filename,
               devlib::StorageDeviceInfo const& info)
    {
        if (_image.use_count() > 1 || _device.use_count() > 1) {
            return false;
        }

        _image  = std::make_shared<QFile>(filename);
        _device = std::make_shared<devlib::StorageDeviceFile>(info);

        return true;
    }

private:
    std::shared_ptr<QFile> _image;
    std::shared_ptr<devlib::StorageDeviceFile> _device;
};


#endif // FLASHERDATAOWNER_H
