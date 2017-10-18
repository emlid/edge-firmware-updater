#ifndef QIOFLASHER_H
#define QIOFLASHER_H

#include <QObject>
#include <QtCore>

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
        QFile& src,
        QFile& dest,
        int blockSize  = DEFAULT_BLOCK_SIZE,
        QCryptographicHash::Algorithm hashAlgo =
            QCryptographicHash::Algorithm::Md5
    );

signals:
    void flashStarted(void);
    void flashCompleted(void);
    void progressChanged(uint progress);
    void flashAborted(FlashingStatus status);

    void checkingCorrectness(void);

private:
    bool _isEquals(QFile& src, QFile& dest, QCryptographicHash::Algorithm hashAlgo);
};

#endif // QIOFLASHER_H
