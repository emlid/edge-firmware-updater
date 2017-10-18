#include "Flasher.h"
#include <memory>
#include <iostream>


Flasher::Flasher(QObject *parent) : QObject(parent) { }


bool Flasher::flash(QFile& src, QFile& dest, int blockSize, QCryptographicHash::Algorithm hashAlgo) {
    auto srcSize  = src.size();
    auto buffer   = std::unique_ptr<char[]>(new char[blockSize]);

    quint64 oldProgress = 0;
    for (qint64 rest = srcSize, rd = 0, i = 0; rest ; rest -= rd, i++) {
        rd = src.read(buffer.get(), blockSize);

        if (rd != blockSize && rd != rest) {
            emit flashAborted(FlashingStatus::READ_FAILED);
            return false;
        }

        qint64 wr = dest.write(buffer.get(), rd);
        if (wr != rd) {
            emit flashAborted(FlashingStatus::WRITE_FAILED);
            return false;
        }

        quint64 progress = (rest * 100) / srcSize;

        if (progress != oldProgress) {
            oldProgress = progress;
            emit progressChanged(100 - (rest * 100) / srcSize);
        }
    }

    emit checkingCorrectness();

    auto result = _isEquals(src, dest, hashAlgo);

    emit flashCompleted();
    return result;
}


bool Flasher::_isEquals(QFile& src, QFile& dest, QCryptographicHash::Algorithm hashAlgo)
{
    QCryptographicHash hashFunction(hashAlgo);

    hashFunction.addData(&src);
    auto srcHash = hashFunction.result();

    qDebug() << "src hash: " << srcHash;

    hashFunction.reset();
    hashFunction.addData(&dest);
    auto destHash = hashFunction.result();

    qDebug() << "dest hash: " << destHash;

    return srcHash.toStdString() == destHash.toStdString();
}
