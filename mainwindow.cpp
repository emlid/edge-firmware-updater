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
    connect(_upgradeController, &FirmwareUpgradeController::refreshDeviceList, this, &MainWindow::refreshListWidget);

    ui->setupUi(this);
    this->alignToCenter();

    ui->lwDeviceList->setSelectionMode(QAbstractItemView::MultiSelection);
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

void MainWindow::on_refreshButton_clicked()
{
    appendStatusLog("Scan for devices...");
    ui->lwDeviceList->clear();
    _upgradeController->clearDeviceList();

    emit _upgradeController->findBoard();
    _upgradeController->print();


}

void MainWindow::on_browseButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
          tr("Open Image"),QDir::homePath(), tr("Image Files (*.img);;All files (*.*)"));
    if (!fileName.isEmpty()) {
        ui->leFileName->setText(fileName);
    }
}


void MainWindow::appendStatusLog(const QString &text, bool critical) {

    if (critical) {
            ui->teLog->append(QString("<font color=\"red\">%1</font>").arg(text));
        } else {
            ui->teLog->append(text);
        }
}

void MainWindow::refreshListWidget()
{
    ui->lwDeviceList->clear();
 /*   QList<StorageDevice*> list = _upgradeController->getDevices();
    QList<StorageDevice*>::iterator i;
    int row = 0;
    for (i = list.begin(); i != list.end(); i++) {
        QProgressBar* progressbar = new QProgressBar(this);
        progressbar->setValue(0);
        progressbar->setTextVisible(1);
progressbar->setStyleSheet("selection-background-color: lightgray");
        progressbar->setFormat((*i)->getNode() + " "+ QString::number(ui->progressBar->value()) + "%");
        ui->lwDeviceList->addItem((*i)->getNode());
        ui->lwDeviceList->setItemWidget(ui->lwDeviceList->item(row), progressbar);

        row++;
    }*/
    QProgressBar* progressbar = new QProgressBar(this);
    progressbar->setValue(0);
    progressbar->setTextVisible(1);
//progressbar->setStyleSheet("selection-background-color: red");
//progressbar->resize(250, 20);
progressbar->setForegroundRole(QPalette::BrightText);
    ui->lwDeviceList->setStyleSheet("QProgressBar::selected {background-color: red;}");
    progressbar->setFormat(" dev/sdb/: "+ QString::number(ui->progressBar->value()) + "%");
    ui->lwDeviceList->addItem("new");
    ui->lwDeviceList->setItemWidget(ui->lwDeviceList->item(0), progressbar);



    // go through _connectedDevices (which is private btw) and create items in the list
}
