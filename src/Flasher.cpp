#include "Flasher.h"
#include <memory>
#include <iostream>
#include <QtConcurrent/QtConcurrent>


Flasher::Flasher(QObject *parent) : QObject(parent) { }


bool Flasher::flash(QFile& src, QFile& dest, int blockSize) {
    emit flashStarted();

    auto srcSize  = src.size();
    auto buffer   = std::unique_ptr<char[]>(new char[blockSize]);

    auto savedProgress = 0l;

    for (auto wroteBytes = 0l; wroteBytes != srcSize;) {

        auto readed = src.read(buffer.get(), blockSize);
        if (readed != blockSize && readed != srcSize - wroteBytes) {
            emit flashFailed(FlashingStatus::READ_FAILED);
            return false;
        }

        auto wrote= dest.write(buffer.get(), readed);
        if (wrote != readed) {
            emit flashFailed(FlashingStatus::WRITE_FAILED);
            return false;
        }

        wroteBytes += wrote;
        auto progress = (wroteBytes * 100l) / srcSize;

        if (progress != savedProgress) {
            savedProgress = progress;
            emit progressChanged(progress);
        }
    }

    emit flashCompleted();
    return true;
}
