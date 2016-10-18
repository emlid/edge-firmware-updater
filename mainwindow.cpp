#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QFileDialog>
#include <QDir>

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

    setupDeviceListWidget();

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
    connect(ui->DeviceList, &QTableWidget::itemSelectionChanged, this, &MainWindow::onDeviceListItemSelectionChanged);
}

void MainWindow::setupDeviceListWidget()
{
    ui->DeviceList->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->DeviceList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->DeviceList->setColumnCount(2);
    ui->DeviceList->horizontalHeader()->setVisible(false);
    ui->DeviceList->verticalHeader()->setVisible(false);
    ui->DeviceList->horizontalHeader()->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    ui->DeviceList->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->DeviceList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->DeviceList->horizontalHeader()->setDefaultSectionSize(60);
    ui->DeviceList->verticalHeader()->setDefaultSectionSize(18);
}

void MainWindow::updateProgressBar(int newValue, int progressBarIndex)
{
     QProgressBar * activeBar = (QProgressBar*)ui->DeviceList->cellWidget(progressBarIndex, 0);
     activeBar->setValue(newValue);
}

void MainWindow::setCancelStartButtonState()
{
    ui->startButton->setEnabled(false);
    ui->cancelButton->setEnabled(false);

    ui->startButton->setEnabled(deviceSelected() && fileSelected() && !flashingInProgress());
    ui->cancelButton->setEnabled(flashingInProgress());
}

void MainWindow::setRefreshButtonState()
{
    ui->refreshButton->setEnabled(!flashingInProgress());
}

bool MainWindow::fileSelected()
{
    return !(ui->FileName->text().isEmpty());
}

bool MainWindow::deviceSelected()
{
    return !(ui->DeviceList->selectionModel()->selectedRows().isEmpty());
}

bool MainWindow::flashingInProgress()
{
    return _upgradeController->flashingInProgress;
}

void MainWindow::onFileNameTextChanged()
{
    setCancelStartButtonState();
}

void MainWindow::onDeviceListItemSelectionChanged()
{
    setCancelStartButtonState();
}


void MainWindow::onRefreshButtonClicked()
{
    ui->refreshButton->setEnabled(false);
    ui->DeviceList->setRowCount(0);
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

    foreach (QModelIndex selectedItem, ui->DeviceList->selectionModel()->selectedRows()){
        _upgradeController->flash(selectedItem.row(), fileName);
        ui->DeviceList->cellWidget(selectedItem.row() ,0)->setEnabled(true);
    }
}

void MainWindow::onCancelButtonClicked()
{
    foreach (QModelIndex selectedItem, ui->DeviceList->selectionModel()->selectedRows()){
         _upgradeController->cancel(selectedItem.row());
         ui->DeviceList->cellWidget(selectedItem.row() ,0)->setEnabled(false);
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

    foreach (StorageDevice *storageDevice, availableDevices) {

        QProgressBar *bar = createProgressBarForDevice(storageDevice);

        ui->DeviceList->insertRow(ui->DeviceList->rowCount());

        /*
         * Enumerating of non empty list starts from 0
         * while rowCount starts from 1. Decrement of rowCount is needed
         * to get index of last row in the list.
         */
        ui->DeviceList->setCellWidget(ui->DeviceList->rowCount() - 1, 0, bar);
    }

    selectFirstDeviceIfNothingElseConnected();
}

QProgressBar*  MainWindow::createProgressBarForDevice(StorageDevice * device)
{
    QProgressBar *bar = new QProgressBar();
    bar->setStyleSheet("QProgressBar {text-align: center;} QProgressBar::chunk:disabled {background-color: gray;}");

    QString text = QString("%1 (%p%)").arg(device->getNode());
    bar->setFormat(text);
    bar->setTextVisible(true);
    bar->setValue(0);

    return bar;
}

void MainWindow::selectFirstDeviceIfNothingElseConnected()
{
    if (ui->DeviceList->rowCount() == 1) {
        ui->DeviceList->selectRow(0);
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

