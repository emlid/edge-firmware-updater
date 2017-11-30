#ifndef QIOFLASHER_H
#define QIOFLASHER_H

#include <QObject>
#include <QtCore>
#include <functional>
#include <memory>

class Flasher : public QObject
{
    Q_OBJECT
public:
    static const int DEFAULT_BLOCK_SIZE = 512;

    enum class FlashingStatus {
       READ_FAILED, WRITE_FAILED
    };

    Flasher(QObject *parent = nullptr);

    bool flash(
        std::shared_ptr<QFile> src,
        std::shared_ptr<QFile> dest,
        int blockSize  = DEFAULT_BLOCK_SIZE
    );

    void setStopCondition(std::function<bool(void)> condition) {
        _stopCondition = condition;
    }

signals:
    void flashStarted(void);
    void flashCompleted(void);
    void progressChanged(uint progress);
    void flashFailed(FlashingStatus status);

private:
    bool _stopRequested(void) {
        return _stopCondition();
    }

    std::function<bool(void)> _stopCondition;
};

#endif // QIOFLASHER_H
