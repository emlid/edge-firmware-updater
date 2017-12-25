#ifndef FLASHERSUBTASK_H
#define FLASHERSUBTASK_H

#include "AbstractSubtask.h"

#include <QtCore>
#include <memory>

class FlasherSubtask : public AbstractSubtask
{
    Q_OBJECT
public:
    explicit FlasherSubtask(std::shared_ptr<QFile> src,
                            std::shared_ptr<QFile> dest,
                            QObject *parent = nullptr);

    virtual ~FlasherSubtask(void) override;

    virtual void run(void) override;

    static QString name(void) {
        return QString("Flasher");
    }

signals:
    void progressChanged(uint value);

private:
    std::shared_ptr<QFile> _src;
    std::shared_ptr<QFile> _dest;
};

#endif // FLASHERSUBTASK_H
