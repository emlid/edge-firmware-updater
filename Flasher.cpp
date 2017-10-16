#include "Flasher.h"
#include <memory>
#include <iostream>


Flasher::Flasher(QObject *parent) : QObject(parent) {  }


bool Flasher::flash(QFile& src, QFile& dest, int blockSize) {
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

    emit flashCompleted();
    return true;
}
