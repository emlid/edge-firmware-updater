#include "flashcontroller.h"

FlashController::FlashController(StorageDevice *device) : deviceForFlash(device)
{

    bar = new QProgressBar(this);
    button = new QPushButton(this);

    bar->setValue(0);
    button->setText("cancel");

    setupStyleSheets();

    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(1);

    layout->addWidget(bar);
    layout->addWidget(button);

    setLayout(layout);
}


void FlashController::setupStyleSheets()
{
    bar->setStyleSheet("QProgressBar {text-align: center;} QProgressBar::chunk:disabled {background-color: gray;}");
    button->setStyleSheet(QString("QPushButton{height: %1px; width: %2px; margin: 0px;}").arg(bar->height()).arg(QFontMetrics(bar->font()).width("cancel") + 10));

}

