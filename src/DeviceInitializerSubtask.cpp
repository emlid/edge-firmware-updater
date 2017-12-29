#include "DeviceInitializerSubtask.h"


DeviceInitializerSubtask::DeviceInitializerSubtask(QObject* parent)
    : AbstractSubtask(DeviceInitializerSubtask::name(), parent)
{ }


DeviceInitializerSubtask::~DeviceInitializerSubtask(void)
{
    qDebug() << "finished";
}


void DeviceInitializerSubtask::run(void)
{
    sendLogMessage("connecting...");

    if (!edge::available()) {
        sendLogMessage("can not connect to the Edge, device not available", Error);
        emit finished(Failed);
        return;
    }

    auto initializer = edge::connect().release();
    sendLogMessage("connected. Initialization...");

    auto optEdgeDevice = initializer();

    if (!optEdgeDevice.present()) {
        sendLogMessage("initialization failed", Error);
        emit finished(Failed);
        return;
    } else {
        sendLogMessage("initialization succeed");
    }

    auto edgeDevice = optEdgeDevice.get();
    auto version = edgeDevice.firmwareVersion();

    sendLogMessage("Firmware version: " + version);

    emit edgeVersion(edgeDevice.firmwareVersion());
    emit deviceAvailable(edgeDevice);
    emit finished(Succeed);
}
