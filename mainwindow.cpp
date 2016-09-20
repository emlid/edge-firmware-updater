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
    connect(_upgradeController, &FirmwareUpgradeController::_updateProgress, this, &MainWindow::updateProgressBar);
    connect(_upgradeController, &FirmwareUpgradeController::changeControlButtonsState, this, &MainWindow::setCancelStartButtonState);
    connect(_upgradeController, &FirmwareUpgradeController::changeControlButtonsState, this, &MainWindow::setRefreshButtonState);
    connect(_upgradeController, &FirmwareUpgradeController::deviceSearchFinished, this, &MainWindow::setRefreshButtonState);

    ui->setupUi(this);
    setCancelStartButtonState();
    this->setFixedSize(this->geometry().width(),this->geometry().height());
    this->alignToCenter();

    ui->lwDeviceList->setSelectionMode(QAbstractItemView::SingleSelection);
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
    return !(ui->lwDeviceList->selectedItems().isEmpty());
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
    ui->lwDeviceList->clear();
    _upgradeController->clearDeviceList();

    _upgradeController->startFindBoardLoop();
}

void MainWindow::on_browseButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
          tr("Open Image"),QDir::homePath(), tr("Image Files (*.img);;All files (*.*)"));
    if (!fileName.isEmpty()) {
        ui->leFileName->setText(fileName);
    }
}

void MainWindow::on_startButton_clicked()
{
    ui->progressBar->setValue(0);
    ui->progressBar->setEnabled(true);
    QString fileName = ui->leFileName->text();
    if (!QFileInfo(fileName).exists()) {
        appendStatusLog(QString("File does not exist"), true);
        return;
    }

    int selectedDeviceIndex = ui->lwDeviceList->currentRow();
    _upgradeController->flash(selectedDeviceIndex, fileName);
}

void MainWindow::on_cancelButton_clicked()
{
    ui->progressBar->setEnabled(false);
    _upgradeController->cancel(ui->lwDeviceList->currentRow());
    ui->lwDeviceList->setCurrentRow(-1);
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
        ui->lwDeviceList->addItem((*i)->getNode());
    }

    if (ui->lwDeviceList->count() == 1) {
        ui->lwDeviceList->setCurrentRow(0);
    }
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
