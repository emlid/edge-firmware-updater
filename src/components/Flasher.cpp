#include "Flasher.h"
#include <memory>
#include <iostream>
#include <QtConcurrent/QtConcurrent>
#include <cstring>


Flasher::Flasher(QObject *parent)
    : QObject(parent), Cancellable()
{ }


bool Flasher::flash(std::shared_ptr<QFile> src, std::shared_ptr<QFile> dest, int blockSize)
{
    emit flashStarted();

    auto srcSize  = src->size();
    auto buffer   = std::unique_ptr<char[]>(new char[blockSize]);

    auto savedProgress = 0;

    for (qint64 wroteBytes = 0; wroteBytes <= srcSize;) {

        auto readed = src->read(buffer.get(), blockSize);
        if (readed != blockSize && readed != srcSize - wroteBytes) {
            emit flashFailed(FlashingStatus::ReadFailed);
            return false;
        } else if (srcSize - wroteBytes == readed) {
            std::memset(buffer.get() + readed, 0, blockSize - readed);
        }

        auto wrote = dest->write(buffer.get(), blockSize);
        if (wrote != blockSize) {
            emit flashFailed(FlashingStatus::WriteFailed);
            return false;
        }

        wroteBytes += wrote;
        qint64 progress = (wroteBytes * 100) / srcSize;

        if (progress != savedProgress) {
            savedProgress = progress;
            emit progressChanged(progress);
        }

        if (_cancelRequested()) {
            _informAboutCancel();
            return true;
        }
    }

    emit flashCompleted();
    return true;
}
