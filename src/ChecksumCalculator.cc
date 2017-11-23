#include <cassert>
#include "ChecksumCalculator.h"

ChecksumCalculator::ChecksumCalculator(QCryptographicHash::Algorithm algo)
    : _hash(algo)
{  }


QByteArray ChecksumCalculator::calculate(QFile &file, qint64 length, int ioBlockSize)
{
    assert(length > 0);
    assert(ioBlockSize >= 0);

    if (!file.isReadable() || !file.isOpen()) {
        qCritical() << "File not readeble or not opened.";
        emit fileReadError();
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

        if (data.size() != ioBlockSize) {
            emit fileReadError();
            return QByteArray();
        }

        _hash.addData(data);
        readed += data.size();
        auto currProgress =
                static_cast<int>((readed * 100) / fileSize);

        if (currProgress != progress) {
            progress = currProgress;
            emit progressChanged(progress);
        }
    }

    auto checksum = _hash.result();

    file.seek(0);
    _hash.reset();

    return checksum;
}
