#ifndef CHECKSUMCALCULATOR_H
#define CHECKSUMCALCULATOR_H

#include <QtCore>

class ChecksumCalculator : public QObject
{
    Q_OBJECT
public:
    explicit ChecksumCalculator(QCryptographicHash::Algorithm algo =
            QCryptographicHash::Algorithm::Md5);

    QByteArray calculate(QFile& file, qint64 length, int ioBlockSize = 4096);

signals:
    void progressChanged(int value);
    void fileReadError(void);

private:
    QCryptographicHash _hash;
};

#endif // CHECKSUMCALCULATOR_H
