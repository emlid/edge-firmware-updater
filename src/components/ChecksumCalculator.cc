#include <cassert>
#include "ChecksumCalculator.h"

ChecksumCalculator::ChecksumCalculator(QCryptographicHash::Algorithm algo)
    : _hash(algo)
{  }


QByteArray ChecksumCalculator::calculate(std::shared_ptr<QFile> file, qint64 length, int ioBlockSize)
{
    Q_ASSERT(file != nullptr);
    Q_ASSERT(length > 0);
    Q_ASSERT(ioBlockSize >= 0);

    if (!file->isReadable() || !file->isOpen()) {
        qCritical() << "File not readeble or not opened.";
        emit fileReadError();
        return QByteArray();
    }

    file->seek(0);

    auto fileSize = length;
    auto progress = 0;

    qint64 readed = 0l;

    while (readed < fileSize) {
        auto data = file->read(ioBlockSize);

        if (readed + ioBlockSize > fileSize) {
            auto truncSize = fileSize - readed;
            data.resize(truncSize);

        } else if (data.size() != ioBlockSize) {
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

        if (_cancelRequested()) {
            file->seek(0);
            _hash.reset();
            _informAboutCancel();
            return QByteArray();
        }
    }

    auto checksum = _hash.result();

    file->seek(0);
    _hash.reset();

    return checksum;
}
