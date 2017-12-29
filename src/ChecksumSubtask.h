#ifndef CHECKSUMSUBTASK_H
#define CHECKSUMSUBTASK_H

#include <QObject>
#include <QFile>
#include <memory>

#include "AbstractSubtask.h"

class ChecksumSubtask : public AbstractSubtask
{
    Q_OBJECT
public:
    explicit ChecksumSubtask(std::shared_ptr<QFile> image,
                             std::shared_ptr<QFile> device,
                             QObject *parent = nullptr);

    virtual ~ChecksumSubtask(void) override;

    virtual void run(void) override;

    static QString name(void) {
        return QString("Checksum");
    }

signals:
    void progressChanged(uint value);

private:
    std::shared_ptr<QFile>   _image;
    std::shared_ptr<QFile>   _device;
};

#endif // CHECKSUMSUBTASK_H
