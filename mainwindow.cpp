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
    ui->teLog->append(QString("Scan for devices..."));
    ui->lwDeviceList->clear();

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
