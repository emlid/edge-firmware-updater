#include "FlasherSubtask.h"
#include "components/Flasher.h"


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

    connect(&flasher, &Flasher::flashFailed,
        [this] (Flasher::FlashingStatus status) {
            if (status == Flasher::FlashingStatus::READ_FAILED) {
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
        sendLogMessage("flashing successfully finished");
        emit finished(Succeed);
    }
}
