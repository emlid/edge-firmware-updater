#ifndef CHECKSUMCALCULATOR_H
#define CHECKSUMCALCULATOR_H

#include <QtCore>
#include <functional>

class ChecksumCalculator : public QObject
{
    Q_OBJECT
public:
    explicit ChecksumCalculator(QCryptographicHash::Algorithm algo =
            QCryptographicHash::Algorithm::Md5);

    QByteArray calculate(QFile* file, qint64 length, int ioBlockSize = 4096);

    void setStopCondition(std::function<bool(void)> condition) {
        _stopCondition = condition;
    }

signals:
    void progressChanged(int value);
    void fileReadError(void);

private:
    bool _stopRequested(void) {
        return _stopCondition();
    }

    QCryptographicHash        _hash;
    std::function<bool(void)> _stopCondition;
};

#endif // CHECKSUMCALCULATOR_H
