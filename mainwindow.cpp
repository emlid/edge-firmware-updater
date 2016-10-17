#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QFileDialog>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    _upgradeController = new FirmwareUpgradeController(this);
    connect(this, &MainWindow::windowShown, _upgradeController, &FirmwareUpgradeController::startFindBoardLoop);
    connect(_upgradeController, &FirmwareUpgradeController::logMessage, this, &MainWindow::appendStatusLog);
    connect(_upgradeController, &FirmwareUpgradeController::updateDeviceList, this, &MainWindow::updateList);
    connect(_upgradeController, &FirmwareUpgradeController::updateProgressBar, this, &MainWindow::updateProgressBar);
    connect(_upgradeController, &FirmwareUpgradeController::changeControlButtonsState, this, &MainWindow::setCancelStartButtonState);
    connect(_upgradeController, &FirmwareUpgradeController::changeControlButtonsState, this, &MainWindow::setRefreshButtonState);
    connect(_upgradeController, &FirmwareUpgradeController::deviceSearchFinished, this, &MainWindow::setRefreshButtonState);

    ui->setupUi(this);
    setCancelStartButtonState();
    this->setFixedSize(this->geometry().width(),this->geometry().height());
    this->alignToCenter();

    setupDeviceListWidget();

    ui->teLog->setReadOnly(true);
    ui->teLog->setTextInteractionFlags(ui->teLog->textInteractionFlags() | Qt::TextSelectableByKeyboard);
    ui->teLog->setVisible(false);

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

void MainWindow::setupDeviceListWidget()
{
    ui->lwDeviceList->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->lwDeviceList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->lwDeviceList->setColumnCount(2);
    ui->lwDeviceList->horizontalHeader()->setVisible(false);
    ui->lwDeviceList->verticalHeader()->setVisible(false);
    ui->lwDeviceList->horizontalHeader()->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    ui->lwDeviceList->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->lwDeviceList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->lwDeviceList->horizontalHeader()->setDefaultSectionSize(60);
    ui->lwDeviceList->verticalHeader()->setDefaultSectionSize(18);
}

void MainWindow::updateProgressBar(int newValue, int progressBarIndex)
{
     QProgressBar * activeBar = (QProgressBar*)ui->lwDeviceList->cellWidget(progressBarIndex, 0);
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
    return !(ui->leFileName->text().isEmpty());
}

bool MainWindow::deviceSelected()
{
    return !(ui->lwDeviceList->selectionModel()->selectedRows().isEmpty());
}

bool MainWindow::flashingInProgress()
{
    return _upgradeController->flashingInProgress;
}

void MainWindow::on_leFileName_textChanged()
{
    setCancelStartButtonState();
}

void MainWindow::on_lwDeviceList_itemSelectionChanged()
{
    setCancelStartButtonState();
}


void MainWindow::on_refreshButton_clicked()
{
    ui->refreshButton->setEnabled(false);
    ui->lwDeviceList->setRowCount(0);
    _upgradeController->clearDeviceList();

    _upgradeController->startFindBoardLoop();
}

void MainWindow::on_browseButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
          "Open Image", QDir::homePath(), "Image Files (*.img);;All files (*.*)");
    if (!fileName.isEmpty()) {
        ui->leFileName->setText(fileName);
    }
}

void MainWindow::on_startButton_clicked()
{
    QString fileName = ui->leFileName->text();
    if (!QFileInfo(fileName).exists()) {
        appendStatusLog(QString("File does not exist"), true);
        return;
    }

    foreach (QModelIndex selectedItem, ui->lwDeviceList->selectionModel()->selectedRows()){
        _upgradeController->flash(selectedItem.row(), fileName);
        ui->lwDeviceList->cellWidget(selectedItem.row() ,0)->setEnabled(true);
    }
}

void MainWindow::on_cancelButton_clicked()
{
    foreach (QModelIndex selectedItem, ui->lwDeviceList->selectionModel()->selectedRows()){
         _upgradeController->cancel(selectedItem.row());
         ui->lwDeviceList->cellWidget(selectedItem.row() ,0)->setEnabled(false);
    }
}

void MainWindow::appendStatusLog(const QString &text, bool critical) {

    if (critical) {
            ui->teLog->append(QString("<font color=\"red\">%1</font>").arg(text));
        } else {
            ui->teLog->append(text);
        }
}

void MainWindow::updateList()
{
    QList<StorageDevice*> availableDevices = _upgradeController->getDevices();
    QList<StorageDevice*>::iterator i;

    for (i = availableDevices.begin(); i != availableDevices.end(); i++) {

        QProgressBar *bar = createProgressBarForDevice(*i);

        ui->lwDeviceList->insertRow(ui->lwDeviceList->rowCount());

        /*
         * Enumerating of non empty list starts from 0
         * while rowCount starts from 1. Decrement of rowCount is needed
         * to get index of last row in the list.
         */
        ui->lwDeviceList->setCellWidget(ui->lwDeviceList->rowCount() - 1, 0, bar);
    }

    if (ui->lwDeviceList->rowCount() == 1) {
        ui->lwDeviceList->selectRow(0);
    }
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

void MainWindow::on_logButton_clicked()
{
    bool LogVisible = ui->teLog->isVisible();
        ui->teLog->setVisible(!LogVisible);
        if (!LogVisible) {
            this->setFixedSize(this->geometry().width(),this->geometry().height() + ui->teLog->height() + 10);
        } else {
            this->setFixedSize(this->geometry().width(),this->geometry().height() - ui->teLog->height() - 10);
        }
}

