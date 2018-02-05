#include "FlasherSubtask.h"
#include "components/Flasher.h"
#include "util/Stopwatch.h"


FlasherSubtask::FlasherSubtask(std::shared_ptr<QFile> src, std::shared_ptr<QFile> dest, QObject *parent)
    : AbstractSubtask(FlasherSubtask::name(), parent), _src(src), _dest(dest)
{ }


FlasherSubtask::~FlasherSubtask(void)
{
    qDebug() << "finished";
}


void FlasherSubtask::run(void)
{
    util::Stopwatch stopwatch;
    Flasher         flasher;

    // Try to open file with Image
    auto succeed = _src->open(QIODevice::ReadOnly);

    if (!succeed) {
        sendLogMessage(": can not open image file", Error);
        emit finished(Failed);
        return;
    }

    // Try to open file, which represent rpi device in filesystem
    succeed = _dest->open(QIODevice::ReadWrite | QIODevice::Unbuffered);

    if (!succeed) {
        emit sendLogMessage("can not open device", Error);
        emit finished(AbstractSubtask::Failed);
        return;
    }

    stopwatch.start();

    connect(&flasher, &Flasher::flashFailed,
        [this] (Flasher::FlashingStatus status) {
            if (status == Flasher::FlashingStatus::ReadFailed) {
                sendLogMessage("image reading failed", Error);
            } else {
                sendLogMessage("writing to the device failed", Error);
            }
        }
    );

    connect(&flasher, &Flasher::progressChanged,
            this,     &FlasherSubtask::progressChanged);

    flasher.setCancelCondition([this](){ return stopRequested(); });

    sendLogMessage("writing new firmware...");

    // Lets flash our rpi device
    auto ioBlockSize = 1 << 16; // 64kb
    auto successful = flasher.flash(_src, _dest, ioBlockSize);

    _src->close();
    if (!successful) {
        sendLogMessage("flashing failed", Error);
        emit finished(Failed);
        return;

    } else if (flasher.wasCancelled()) {
        sendLogMessage("flashing cancelled");
        emit finished(Cancelled);

    } else {
        auto timeElapsed   = stopwatch.elapsed().asQString();
        auto profilingData =
                QString("Time elapsed: ") + timeElapsed + " minutes.";

        sendLogMessage(QString("flashing successfully finished. ") + profilingData);
        emit finished(Succeed);
    }
}
