#ifndef FIRMWAREUPGRADECONTROLLER_H
#define FIRMWAREUPGRADECONTROLLER_H

#include <QObject>

class FirmwareUpgradeController : public QObject
{
    Q_OBJECT
public:
    explicit FirmwareUpgradeController(QObject *parent = 0);

signals:

public slots:
};

#endif // FIRMWAREUPGRADECONTROLLER_H