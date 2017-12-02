#ifndef QIOFLASHER_H
#define QIOFLASHER_H

#include <QObject>
#include <QtCore>
#include <functional>
#include <memory>

#include "Cancellable.h"

class Flasher : public QObject, public Cancellable
{
    Q_OBJECT
public:
    static const int DEFAULT_BLOCK_SIZE = 512;

    enum class FlashingStatus {
       READ_FAILED, WRITE_FAILED
    };

    explicit Flasher(QObject *parent = nullptr);

    bool flash(
        std::shared_ptr<QFile> src,
        std::shared_ptr<QFile> dest,
        int blockSize  = DEFAULT_BLOCK_SIZE
    );

signals:
    void flashStarted(void);
    void flashCompleted(void);
    void progressChanged(uint progress);
    void flashFailed(FlashingStatus status);
};

#endif // QIOFLASHER_H
