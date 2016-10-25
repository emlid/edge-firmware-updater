#ifndef FLASHCONTROLLER_H
#define FLASHCONTROLLER_H

#include <QWidget>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QSignalMapper>
#include <storagedevice.h>

class FlashController : public QWidget
{
    Q_OBJECT

public:
    explicit FlashController(StorageDevice *device);

    void setBarText(QString text) {QString format = QString("%1 (%p%)").arg(text);bar->setFormat(format);}
    void setBarValue(int value) {bar->setValue(value);}
    void setBarEnabled(bool state) {bar->setEnabled(state);}
    void setupStyleSheets();
    void mapImageFileName(QString fileName);
    void setButtonEnabled(bool arg);
    void setBoxChecked(bool arg);

signals:
    void boxStateChanged(bool state);

public slots:
    void onFlasingStopped();
    void onFlasingStarted();
    void convertBoxStateToBool(int state);

private:
    QProgressBar *bar;
    QPushButton *button;
    QCheckBox *box;

    StorageDevice *deviceForFlash;
    QSignalMapper *mapperForImageName;
};

#endif // FLASHCONTROLLER_H
