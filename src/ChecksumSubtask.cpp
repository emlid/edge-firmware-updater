#include "ChecksumSubtask.h"
#include "components/ChecksumCalculator.h"


ChecksumSubtask::ChecksumSubtask(std::shared_ptr<QFile> image,
                                 std::shared_ptr<QFile> device,
                                 QObject *parent)
    : AbstractSubtask(ChecksumSubtask::name(), parent), _image(image), _device(device)
{ }


ChecksumSubtask::~ChecksumSubtask(void)
{
    qDebug() << "finished";
}


void ChecksumSubtask::run(void)
{
    Q_ASSERT(_image  != nullptr);
    Q_ASSERT(_device != nullptr);

    using CsSubtask  = ChecksumSubtask;
    using Calculator = ChecksumCalculator;

    ChecksumCalculator calc;

    calc.setCancelCondition([this](){ return stopRequested(); });
    QObject::connect(&calc, &Calculator::progressChanged, this,  &CsSubtask::progressChanged);

    sendLogMessage("started... please wait");
    sendLogMessage("compute image checksum...");
    auto imgChecksum = calc.calculate(_image, _image->size());

    if (calc.wasCancelled()) {
        sendLogMessage("cancelled");
        emit finished(Cancelled);
        return;
    }

    sendLogMessage("compute device checksum...");
    auto deviceChecksum = calc.calculate(_device, _image->size());

    if (calc.wasCancelled()) {
        sendLogMessage("cancelled");
        emit finished(Cancelled);
        return;
    }

    if (imgChecksum != deviceChecksum) {
        sendLogMessage("Image incorrectly wrote.", Warning);
        emit finished(Failed);
    } else {
        sendLogMessage("Image correctly wrote.");
        emit finished(Succeed);
    }
}
