#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "libusb-1.0/libusb.h"
#include <libudev.h>

#include <QDesktopWidget>
#include <QFileDialog>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    _upgradeController = new FirmwareUpgradeController(this);
    ui->setupUi(this);
    this->alignToCenter();

    ui->lwDeviceList->setSelectionMode(QAbstractItemView::MultiSelection);

    on_refreshButton_clicked();
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

    const char* vid="0a5c";
    struct udev *udev = udev_new();
    struct udev_enumerate* enumerate;
    struct udev_list_entry *devices;
    struct udev_list_entry *entry;
    const char* path;
    struct udev_device* block;
    bool noDevice = true;

    enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "disk");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(entry, devices) {
        path = udev_list_entry_get_name(entry);
        block = udev_device_new_from_syspath(udev, path);

        if (block) {
            const char* vendorID = udev_device_get_property_value(block, "ID_VENDOR_ID");
            if (vendorID != NULL && strcmp(vendorID, vid) == 0) {
                const char* modelID = udev_device_get_property_value(block, "ID_MODEL_ID");
                const char* devnode = udev_device_get_devnode(block);

                ui->lwDeviceList->addItem(QString("%1 : %2 -> %3").arg(vendorID).arg(modelID).arg(devnode));
                ui->teLog->append(QString("  %1").arg(devnode));
                noDevice = false;
                udev_device_unref(block);
                break;
            }
            udev_device_unref(block);
        }
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    if (noDevice) {
        ui->teLog->append(QString("No device found!"));
    }
}

void MainWindow::on_browseButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
          tr("Open Image"),QDir::homePath(), tr("Image Files (*.img);;All files (*.*)"));
    if (!fileName.isEmpty()) {
        ui->leFileName->setText(fileName);
    }
}
