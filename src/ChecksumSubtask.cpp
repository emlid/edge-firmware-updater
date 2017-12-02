#include "ChecksumSubtask.h"
#include "components/ChecksumCalculator.h"


ChecksumSubtask::ChecksumSubtask(std::shared_ptr<QFile> image,
                                 std::shared_ptr<QFile> device,
                                 QObject *parent)
    : AbstractSubtask("ChecksumSubtask", parent), _image(image), _device(device)
{ }


ChecksumSubtask::~ChecksumSubtask(void)
{
    qDebug() << _subtaskMsg("finished");
}


void ChecksumSubtask::run(void)
{
    Q_ASSERT(_image  != nullptr);
    Q_ASSERT(_device != nullptr);

    using CsSubtask  = ChecksumSubtask;
    using Calculator = ChecksumCalculator;
    using CsState    = states::CheckingCorrectnessState;
    using StType     = states::StateType;

    ChecksumCalculator calc;

    calc.setCancelCondition([this](){ return _stopRequested(); });

    QObject::connect(&calc, &Calculator::progressChanged, this,  &CsSubtask::progressChanged);
    QObject::connect(&calc, &Calculator::fileReadError,
        [this] () { emit stateChanged(CsState::ReadingFailed, StType::Error); });

    qInfo() << _subtaskMsg("started...");
    emit stateChanged(CsState::CheckingCorrectnessStarted);

    emit stateChanged(CsState::ComputeImageChecksum);
    auto imgChecksum = calc.calculate(_image, _image->size());

    if (calc.wasCancelled()) {
        emit stateChanged(CsState::CheckingCancelled);
        emit finished(false);
        return;
    }

    emit stateChanged(CsState::ComputeDeviceChecksum);
    auto deviceChecksum = calc.calculate(_device, _image->size());

    if (calc.wasCancelled()) {
        emit stateChanged(CsState::CheckingCancelled);
        emit finished(false);
        return;
    }

    if (imgChecksum != deviceChecksum) {
        qWarning() << _subtaskMsg("Image incorrectly wrote.");
        emit stateChanged(CsState::ImageUncorrectlyWrote, StType::Warning);
        emit finished(false);
    } else {
        qInfo() << _subtaskMsg("Image correctly wrote.");
        emit stateChanged(CsState::ImageCorrectlyWrote);
        emit finished();
    }
}
