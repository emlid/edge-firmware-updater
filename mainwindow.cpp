#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include "flashcontroller.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _upgradeController = new FirmwareUpgradeController(this);
    connect(this, &MainWindow::windowShown, _upgradeController, &FirmwareUpgradeController::startFindBoardLoop);
    connect(_upgradeController, &FirmwareUpgradeController::logMessage, this, &MainWindow::appendStatusLog);
    connect(_upgradeController, &FirmwareUpgradeController::updateDeviceList, this, &MainWindow::updateList);
    connect(_upgradeController, &FirmwareUpgradeController::updateProgressBar, this, &MainWindow::updateProgressBar);
    connect(_upgradeController, &FirmwareUpgradeController::changeControlButtonsState, this, &MainWindow::setCancelStartButtonState);
    connect(_upgradeController, &FirmwareUpgradeController::changeControlButtonsState, this, &MainWindow::setRefreshButtonState);
    connect(_upgradeController, &FirmwareUpgradeController::deviceSearchFinished, this, &MainWindow::setRefreshButtonState);

    connectGuiSignalsToSlots();

    setCancelStartButtonState();
    this->setFixedSize(this->geometry().width(),this->geometry().height());
    this->alignToCenter();

    ui->DeviceList->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->DeviceList->setSpacing(1);

    ui->Log->setReadOnly(true);
    ui->Log->setTextInteractionFlags(ui->Log->textInteractionFlags() | Qt::TextSelectableByKeyboard);
    ui->Log->setVisible(false);

    ui->refreshButton->setEnabled(false);
}

void MainWindow::alignToCenter()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    this->move(x, y);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::show()
{
   QMainWindow::show();
   QApplication::processEvents();
   emit windowShown();
}

void MainWindow::connectGuiSignalsToSlots()
{
    connect(ui->refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshButtonClicked);
    connect(ui->browseButton, &QPushButton::clicked, this, &MainWindow::onBrowseButtonClicked);
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::onStartButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &MainWindow::onCancelButtonClicked);
    connect(ui->logButton, &QPushButton::clicked, this, &MainWindow::onLogButtonClicked);
    connect(ui->FileName, &QLineEdit::textChanged, this, &MainWindow::onFileNameTextChanged);
    connect(ui->DeviceList, &QListWidget::itemSelectionChanged, this, &MainWindow::onDeviceListItemSelectionChanged);
    connect(ui->selectAllDevices ,&QCheckBox::stateChanged, this, &MainWindow::onCheckBoxStateChanged);
}

void MainWindow::updateProgressBar(int newValue, int progressBarIndex)
{
    QListWidgetItem *activeItem = ui->DeviceList->item(progressBarIndex);
    FlashController *activeBar = (FlashController*)ui->DeviceList->itemWidget(activeItem);
    activeBar->setBarValue(newValue);
}

void MainWindow::setCancelStartButtonState()
{
    ui->startButton->setEnabled(false);
    ui->cancelButton->setEnabled(false);

    ui->startButton->setEnabled(inactiveDeviceSelected() && fileSelected());
    ui->cancelButton->setEnabled(activeDeviceSelected());
}

void MainWindow::setRefreshButtonState()
{
    ui->refreshButton->setEnabled(!flashingInProgress());
}

bool MainWindow::fileSelected()
{
    return !(ui->FileName->text().isEmpty());
}

bool MainWindow::flashingInProgress()
{
    return _upgradeController->flashingInProgress();
}

bool MainWindow::inactiveDeviceSelected()
{
    QList<StorageDevice*> connectedDevices = _upgradeController->getDevices();
    foreach (QModelIndex selectedItem, ui->DeviceList->selectionModel()->selectedIndexes()){
        if (!connectedDevices.at(selectedItem.row())->inUse) {
            return true;
        }
    }
    return false;
}

bool MainWindow::activeDeviceSelected()
{
    QList<StorageDevice*> connectedDevices = _upgradeController->getDevices();
    foreach (QModelIndex selectedItem, ui->DeviceList->selectionModel()->selectedIndexes()){
        if (connectedDevices.at(selectedItem.row())->inUse) {
            return true;
        }
    }
    return false;
}

void MainWindow::onFileNameTextChanged()
{
    setCancelStartButtonState();
    mapFileNameForEachDevice(ui->FileName->text());
}

void MainWindow::mapFileNameForEachDevice(QString fileName)
{
    for(int i = 0; i < ui->DeviceList->count(); i++) {
        QListWidgetItem *currentItem = ui->DeviceList->item(i);
        FlashController *currentController = (FlashController*)ui->DeviceList->itemWidget(currentItem);
        currentController->mapImageFileName(fileName);

        if (!fileName.isEmpty()) {
            currentController->setButtonEnabled(true);
        } else {
            currentController->setButtonEnabled(false);
        }
    }
}

void MainWindow::onDeviceListItemSelectionChanged()
{
    setCancelStartButtonState();
    updateCheckBoxes();
}

void MainWindow::updateCheckBoxes()
{
    for(int i = 0; i < ui->DeviceList->count(); i++) {
        QListWidgetItem *item = ui->DeviceList->item(i);
        FlashController *widget = (FlashController*)ui->DeviceList->itemWidget(item);

        if (item->isSelected()){
            widget->setBoxChecked(true);
        } else {
            widget->setBoxChecked(false);
        }
    }
}

void MainWindow::onRefreshButtonClicked()
{
    ui->refreshButton->setEnabled(false);
    ui->DeviceList->clear();
    _upgradeController->clearDeviceList();

    _upgradeController->startFindBoardLoop();
}

void MainWindow::onBrowseButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
          "Open Image", QDir::homePath(), "Image Files (*.img);;All files (*.*)");
    if (!fileName.isEmpty()) {
        ui->FileName->setText(fileName);
    }
}

void MainWindow::onStartButtonClicked()
{
    QString fileName = ui->FileName->text();
    if (!QFileInfo(fileName).exists()) {
        appendStatusLog(QString("File does not exist"), true);
        return;
    }

    foreach (QModelIndex selectedItem, ui->DeviceList->selectionModel()->selectedIndexes()){
        _upgradeController->flash(selectedItem.row(), fileName);

        FlashController *selected = (FlashController *)ui->DeviceList->indexWidget(selectedItem);
        selected->setBarEnabled(true);
    }
}

void MainWindow::onCancelButtonClicked()
{
    foreach (QModelIndex selectedItem, ui->DeviceList->selectionModel()->selectedIndexes()){
         _upgradeController->cancel(selectedItem.row());

         FlashController *selected = (FlashController *)ui->DeviceList->indexWidget(selectedItem);
         selected->setBarEnabled(false);
    }
}

void MainWindow::appendStatusLog(const QString &text, bool critical) {

    if (critical) {
            ui->Log->append(QString("<font color=\"red\">%1</font>").arg(text));
        } else {
            ui->Log->append(text);
        }
}

void MainWindow::updateList()
{
    QList<StorageDevice*> availableDevices = _upgradeController->getDevices();

    enableSelectAllForDeviceList(availableDevices);

    createWidgetForEachDevice(availableDevices);

    selectFirstDeviceIfNothingElseConnected();
}

void MainWindow::enableSelectAllForDeviceList(QList<StorageDevice*> deviceList)
{
    if (deviceList.isEmpty()) {
        ui->selectAllDevices->setEnabled(false);
    } else {
        ui->selectAllDevices->setChecked(false);
        ui->selectAllDevices->setEnabled(true);
    }
}

void MainWindow::createWidgetForEachDevice(QList<StorageDevice*> deviceList)
{
    foreach (StorageDevice *storageDevice, deviceList) {

        FlashController *controllerForSingleDevice = new FlashController(storageDevice);
        if (!ui->FileName->text().isEmpty()){
            controllerForSingleDevice->mapImageFileName(ui->FileName->text());
            controllerForSingleDevice->setButtonEnabled(true);
        }

        controllerForSingleDevice->setBarText(storageDevice->getNode());
        QListWidgetItem *item = new QListWidgetItem();

        ui->DeviceList->addItem(item);
        ui->DeviceList->setItemWidget(item, controllerForSingleDevice);

        connect(controllerForSingleDevice, &FlashController::boxStateChanged, [item](bool state){item->setSelected(state);});
    }
}

void MainWindow::selectFirstDeviceIfNothingElseConnected()
{
    if (ui->DeviceList->count() == 1) {
        ui->DeviceList->setCurrentRow(0);
    }
}

void MainWindow::onLogButtonClicked()
{
    bool LogVisible = ui->Log->isVisible();
        ui->Log->setVisible(!LogVisible);
        if (!LogVisible) {
            this->setFixedSize(this->geometry().width(),this->geometry().height() + ui->Log->height() + 10);
        } else {
            this->setFixedSize(this->geometry().width(),this->geometry().height() - ui->Log->height() - 10);
        }
}

void MainWindow::onCheckBoxStateChanged(int state)
{
    if (state > 0) {
        ui->DeviceList->selectAll();
    } else {
        ui->DeviceList->clearSelection();
    }
}
