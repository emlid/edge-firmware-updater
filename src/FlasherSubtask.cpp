#include "FlasherSubtask.h"
#include "components/Flasher.h"

#include <chrono>


FlasherSubtask::FlasherSubtask(std::shared_ptr<QFile> src, std::shared_ptr<QFile> dest, QObject *parent)
    : AbstractSubtask(FlasherSubtask::name(), parent), _src(src), _dest(dest)
{ }


FlasherSubtask::~FlasherSubtask(void)
{
    qDebug() << "finished";
}


void FlasherSubtask::run(void)
{
    Flasher flasher;

    using stopwatch = std::chrono::high_resolution_clock;
    auto startTime = stopwatch::now();

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
    }

    if (flasher.wasCancelled()) {
        sendLogMessage("flashing cancelled");
        emit finished(Cancelled);
    } else {
        auto finishTime = stopwatch::now();
        auto workingTime = finishTime - startTime;

        auto chronoSeconds = std::chrono::
                duration_cast<std::chrono::seconds>(workingTime);

        auto mins = chronoSeconds.count() / 60;
        auto secs = chronoSeconds.count() - mins * 60;

        auto leadSymbol = QChar('0');
        auto leadCount  = 2;
        auto base       = 10;
        auto profilingData = QString("time elapsed: %1:%2 minutes")
                                 .arg(mins, leadCount, base, leadSymbol)
                                 .arg(secs, leadCount, base, leadSymbol);

        sendLogMessage(profilingData);

        sendLogMessage("flashing successfully finished");
        emit finished(Succeed);
    }
}
