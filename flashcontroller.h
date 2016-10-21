#ifndef FLASHCONTROLLER_H
#define FLASHCONTROLLER_H

#include <QWidget>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QPushButton>
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

signals:

public slots:

private:
    QProgressBar *bar;
    QPushButton *button;
};

#endif // FLASHCONTROLLER_H
