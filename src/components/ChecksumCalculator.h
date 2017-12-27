#ifndef CHECKSUMCALCULATOR_H
#define CHECKSUMCALCULATOR_H

#include <QtCore>
#include <functional>
#include <memory>

#include "Cancellable.h"

class ChecksumCalculator
        : public QObject, public Cancellable
{
    Q_OBJECT
public:
    explicit ChecksumCalculator(QCryptographicHash::Algorithm algo =
            QCryptographicHash::Algorithm::Md5);

    QByteArray calculate(std::shared_ptr<QFile> file, qint64 length, int ioBlockSize = 65536);

signals:
    void progressChanged(int value);
    void fileReadError(void);

private:
    QCryptographicHash        _hash;
};

#endif // CHECKSUMCALCULATOR_H
