#include "ChecksumCalculator.h"

ChecksumCalculator::ChecksumCalculator(QCryptographicHash::Algorithm algo)
    : _hash(algo)
{  }


QByteArray ChecksumCalculator::calculate(QFile &file, qint64 length, int ioBlockSize)
{
    if (!file.isReadable()) {
        qWarning() << "File not readeble";
        emit errorOcurred();
        return QByteArray();
    }

    file.seek(0);

    auto fileSize = length;
    auto progress = 0;

    qint64 readed = 0l;

    while (readed < fileSize) {
        if (readed + ioBlockSize > fileSize) {
            ioBlockSize = fileSize - readed;
        }

        auto data = file.read(ioBlockSize);
        _hash.addData(data);

        readed += data.size();

        if (readed == 0) {
            break;
        }

        auto currProgress = static_cast<int>((readed * 100) / fileSize);
        if (currProgress != progress) {
            progress = currProgress;
            qInfo() << "Progress " << progress << "%";
            emit progressChanged(progress);
        }
    }

    qInfo() << "File size: " << fileSize;
    qInfo() << "Readed size: " << readed;

    auto checksum = _hash.result();

    file.seek(0);
    _hash.reset();

    return checksum;
}
