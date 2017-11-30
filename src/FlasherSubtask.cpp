#include "FlasherSubtask.h"
#include "components/Flasher.h"


FlasherSubtask::FlasherSubtask(std::shared_ptr<QFile> src, std::shared_ptr<QFile> dest, QObject *parent)
    : AbstractSubtask("FlasherSubtask", parent), _src(src), _dest(dest)
{ }


FlasherSubtask::~FlasherSubtask(void)
{
    qDebug() << _subtaskMsg("finished");
}


void FlasherSubtask::run(void)
{
    using FlState = states::FlasherState;
    using StType  = states::StateType;

    Flasher flasher;

    connect(&flasher, &Flasher::flashStarted,
        [this] () { emit stateChanged(FlState::FlasherStarted); });

    connect(&flasher, &Flasher::flashFailed,
        [this] (Flasher::FlashingStatus status) {
            if (status == Flasher::FlashingStatus::READ_FAILED) {
                emit stateChanged(FlState::FlasherImageReadingFailed, StType::Error);
            } else {
                emit stateChanged(FlState::FlasherDeviceWritingFailed, StType::Error);
            }
        }
    );

    connect(&flasher, &Flasher::progressChanged, this, &FlasherSubtask::progressChanged);

    flasher.setStopCondition([this](){ return _stopRequested(); });

    // Lets flash our rpi device
    auto ioBlockSize = 1 << 16; // 64kb
    auto successful = flasher.flash(_src, _dest, ioBlockSize);

    if (!successful) {
        qCritical() << _subtaskMsg("Flashing failed");
        emit stateChanged(FlState::FlasherFailed, StType::Error);
        emit finished(false);
        return;
    }

    emit stateChanged(FlState::FlasherFinished);
    emit finished();
}
