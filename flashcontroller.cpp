#include "flashcontroller.h"

FlashController::FlashController(StorageDevice *device) : deviceForFlash(device)
{

    bar = new QProgressBar(this);
    button = new QPushButton(this);
    box = new QCheckBox(this);

    bar->setValue(0);
    button->setText("start");
    button->setEnabled(false);

    setupStyleSheets();

    mapperForImageName = new QSignalMapper(this);
    connect(mapperForImageName, static_cast<void(QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped), deviceForFlash, &StorageDevice::flash);
    connect(button, SIGNAL(clicked()), mapperForImageName, SLOT(map()));

    connect(deviceForFlash, &StorageDevice::flashingStarted, this, FlashController::onFlasingStarted);
    connect(deviceForFlash, &StorageDevice::flashComplete, this, FlashController::onFlasingStopped);


    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(3);

    layout->addWidget(box);
    layout->addWidget(bar);
    layout->addWidget(button);

    setLayout(layout);
}


void FlashController::setupStyleSheets()
{
    bar->setStyleSheet("QProgressBar {text-align: center;} QProgressBar::chunk:disabled {background-color: gray;}");
    button->setStyleSheet(QString("QPushButton{height: %1px; width: %2px; margin: 0px;}").arg(bar->height()).arg(QFontMetrics(bar->font()).width("cancel") + 10));

}

void FlashController::onFlasingStopped()
{
    setBarEnabled(false);
    button->setText("start");

    disconnect(button, &QPushButton::clicked, deviceForFlash, &StorageDevice::cancel);
    connect(button, SIGNAL(clicked()), mapperForImageName, SLOT(map()));
}


void FlashController::onFlasingStarted()
{
    setBarEnabled(true);
    button->setText("cancel");

    disconnect(button, SIGNAL(clicked()), mapperForImageName, SLOT(map()));
    connect(button, &QPushButton::clicked, deviceForFlash, &StorageDevice::cancel);
}


void FlashController::mapImageFileName(QString fileName)
{
    mapperForImageName->setMapping(button, fileName);
}

void FlashController::setButtonEnabled(bool arg)
{
    button->setEnabled(arg);
}

void FlashController::setBoxChecked(bool arg)
{
    box->setChecked(arg);
}
