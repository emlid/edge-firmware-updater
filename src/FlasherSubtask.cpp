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

    // Lets flash our rpi device
    auto ioBlockSize = 1 << 16; // 64kb
    auto successful = flasher.flash(_src, _dest, ioBlockSize);

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
