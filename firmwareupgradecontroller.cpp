#include "firmwareupgradecontroller.h"
#include <QDebug>

FirmwareUpgradeController::FirmwareUpgradeController(QObject *parent) : QObject(parent)
{
    qDebug() << "firmwareUpgradeController constructor";
}

void FirmwareUpgradeController::startFindDevices()
{
    qDebug() << "start Find defices";
}
