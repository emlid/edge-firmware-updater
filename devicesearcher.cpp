#include "firmwareupgradecontroller.h"

void DeviceSearcher::startFindBoardLoop()
{
    emit foundDevice(228, 228, QString("dummy"));
}
