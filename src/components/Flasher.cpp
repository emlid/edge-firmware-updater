#include "Flasher.h"
#include <memory>
#include <iostream>
#include <QtConcurrent/QtConcurrent>


Flasher::Flasher(QObject *parent)
    : QObject(parent),
      _stopCondition([](){return false;})
{ }


bool Flasher::flash(QFile& src, QFile& dest, int blockSize) {
    emit flashStarted();

    auto srcSize  = src.size();
    auto buffer   = std::unique_ptr<char[]>(new char[blockSize]);

    auto savedProgress = 0;

    for (qint64 wroteBytes = 0; wroteBytes != srcSize;) {

        auto readed = src.read(buffer.get(), blockSize);
        if (readed != blockSize && readed != srcSize - wroteBytes) {
            emit flashFailed(FlashingStatus::READ_FAILED);
            return false;
        }

        auto wrote = dest.write(buffer.get(), readed);
        if (wrote != readed) {
            emit flashFailed(FlashingStatus::WRITE_FAILED);
            return false;
        }

        wroteBytes += wrote;
        qint64 progress = (wroteBytes * 100) / srcSize;

        if (progress != savedProgress) {
            savedProgress = progress;
            emit progressChanged(progress);
        }

        if (_stopRequested()) {
            return true;
        }
    }

    emit flashCompleted();
    return true;
}
