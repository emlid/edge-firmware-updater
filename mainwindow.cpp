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

    ui->setupUi(this);
    setCancelStartButtonState();
    this->setFixedSize(this->geometry().width(),this->geometry().height());
    this->alignToCenter();

    ui->lwDeviceList->setSelectionMode(QAbstractItemView::SingleSelection);
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

    ui->startButton->setEnabled(deviceSelected() && fileSelected());
}

bool MainWindow::fileSelected()
{
    return !(ui->leFileName->text().isEmpty());
}

bool MainWindow::deviceSelected()
{
    return !(ui->lwDeviceList->selectedItems().isEmpty());
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
    ui->lwDeviceList->clear();
    _upgradeController->clearDeviceList();

    emit _upgradeController->findBoard();


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
    QString fileName = ui->leFileName->text();
    int selectedDeviceIndex = ui->lwDeviceList->currentRow();
    _upgradeController->flash(selectedDeviceIndex, fileName);
}

void MainWindow::on_cancelButton_clicked()
{
    _upgradeController->cancel(ui->lwDeviceList->currentRow());
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
