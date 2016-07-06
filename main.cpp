#include <QCoreApplication>
#include <QtSerialPort/QSerialPortInfo>
#include <QSerialPortInfo>
#include <QtDebug>
#include <QStorageInfo>
#include <QByteArrayList>
#include <QThread>
#include "libusb-1.0/libusb.h"
#include <stdio.h>
#include <stdlib.h>
#include <libudev.h>
#include <string.h>
#include <QtConcurrent/QtConcurrent>


#include <dd.h>
#include <rpiboot.h>
#include <flashing_parameters.h>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QTextStream>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <iostream>
using namespace std;

#define VENDOR_ID       0xa5c
#define VENDOR_ID_CHAR   "0a5c"
#define PRODUCT_ID1     0x2763
#define PRODUCT_ID2     0x2764



static struct udev_device* get_child(struct udev* udev, struct udev_device* parent, const char* subsystem) {
    struct udev_device* child = NULL;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices;
    struct udev_list_entry *entry;
    const char *path;

    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_parent(enumerate, parent);
    udev_enumerate_add_match_subsystem(enumerate, subsystem);
    udev_enumerate_scan_devices(enumerate);

    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(entry, devices) {
        path = udev_list_entry_get_name(entry);
        child = udev_device_new_from_syspath(udev, path);
        if (strcmp(udev_device_get_property_value(child, "ID_VENDOR_ID"), VENDOR_ID_CHAR) == 0)
            break;
    }

    udev_enumerate_unref(enumerate);
    return child;
}


static void findMountedDeviceByVid(QString* devnode, const char* vid) {
    struct udev *udev = udev_new();
    struct udev_enumerate* enumerate;
    struct udev_list_entry *devices;
    struct udev_list_entry *entry;
    const char* path;
    struct udev_device* scsi;
    struct udev_device* block;

    enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(enumerate, "scsi");
    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "scsi_device");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(entry, devices) {
        path = udev_list_entry_get_name(entry);
        scsi = udev_device_new_from_syspath(udev, path);

        block = get_child(udev, scsi, "block");

        if (block) {
            cout << "Found device:" << endl;
            cout << " VID/PID: " << udev_device_get_property_value(block, "ID_VENDOR_ID");
            cout << "/" << udev_device_get_property_value(block, "ID_MODEL_ID") << endl;
            cout << " Node Path: " << udev_device_get_devnode(block) << endl;

            if (strcmp(udev_device_get_property_value(block, "ID_VENDOR_ID"), vid) == 0) {
                devnode->append(udev_device_get_devnode(block));
                udev_device_unref(block);
                udev_device_unref(scsi);

                break;
            }
            udev_device_unref(block);
        }
        udev_device_unref(scsi);

    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}



void startRpiBoot(QThread *thread){
    thread->setObjectName("rpiboot Thread");
    QObject::connect(thread, &QThread::started, rpiboot);
    thread->start();
}


void catchNewDeviceByVid(QString* devnode, const char* vid){

    struct udev* udev = udev_new();
    struct udev_device* dev;
    struct udev_monitor* mon;
    int fd;
    struct timeval tv;
    int ret;

    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "disk");
    udev_monitor_enable_receiving(mon);
    fd = udev_monitor_get_fd(mon);

    while (1) {
        fd_set fds;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(fd+1, &fds, NULL, NULL, &tv);

        if (ret > 0 && FD_ISSET(fd, &fds)) {
            dev = udev_monitor_receive_device(mon);
            if (!strcmp( udev_device_get_action(dev), "add") && !strcmp( udev_device_get_property_value(dev, "ID_VENDOR_ID"), vid)) {
                cout << "\nDetected storage device:\n";
                cout << "   Node: " << udev_device_get_devnode(dev) << endl;
                cout << "   Subsystem: " << udev_device_get_subsystem(dev) << endl;
                cout << "   Devtype: " << udev_device_get_devtype(dev) << endl;
                devnode->append(udev_device_get_devnode(dev));
                break;

            }
        }
    }

    udev_device_unref(dev);
    udev_unref(udev);
}



int parseCommandLineArguments(const QStringList & arguments, struct FlashingParameters* param){
    bool parseResult;
    QCommandLineParser parser;
    parser.addHelpOption();

    QCommandLineOption pathToImage(QStringList() << "I" << "image", QCoreApplication::translate("main", "Path to image") \
                                   ,QCoreApplication::translate("main", "path"));
    parser.addOption(pathToImage);

    parseResult = parser.parse(arguments);
    if (!parseResult) {
        cerr << qPrintable(parser.errorText()) << endl;
        cout << qPrintable(parser.helpText()) << endl;
        return 1;
    }

    bool gotPath = parser.isSet(pathToImage);

    if (!gotPath) {
        cerr << "Error: an argument is required." << endl;
        cout << qPrintable(parser.helpText()) << endl;
        return 1;
    }

    QString path = parser.value(pathToImage);
    QRegExp rx("*/*.img");
    rx.setPatternSyntax(QRegExp::Wildcard);

    if (!rx.exactMatch(path)) {
        cerr << "Error: invalid path or file type. Should be /path/to/image.img" << endl;
        return 1;
    }

    QFile file(path);

    if( !file.exists() )
    {
        cerr << "Error: file does not exist" << endl;
        return 1;
    }

    param->inputFile.append(path);
    return 0;
}



int getDeviceNodePath(QString *foundedDevice){

    libusb_context *ctx;

    int ret = libusb_init(&ctx);
    if (ret) {
        cerr << "Failed to initialise libUSB" << endl;
        return ret;
    }

    libusb_device **devs;

    ret = libusb_get_device_list(NULL, &devs);
    if (ret < 0){
        cerr << "Failed to get devices list" << endl;
        return ret;
    }

    libusb_device *dev;

    int i = 0;
    while ( (dev = devs[i]) != NULL ) {

        struct libusb_device_descriptor desc;

        if (libusb_get_device_descriptor(dev, &desc) < 0) {
            cerr << "Can't get device descriptor" << endl;
            continue;
        }

        if (desc.idVendor == VENDOR_ID) {

            if (desc.idProduct == PRODUCT_ID1 || desc.idProduct == PRODUCT_ID2) {

                QThread rpibootThread;
                startRpiBoot(&rpibootThread);

                catchNewDeviceByVid(foundedDevice, VENDOR_ID_CHAR);

            } else {

                findMountedDeviceByVid(foundedDevice, VENDOR_ID_CHAR);

            }

            break;
        }
        i++;
    }

    if(foundedDevice->size() == 3) {
        cerr << "Device not found." << endl;
        return 1;
    }

    libusb_exit(ctx);
    return 0;
}


int main(int argc, char *argv[])
{
    int ret = 0;
    struct FlashingParameters flashParams;

    QCoreApplication a(argc, argv);

    ret = parseCommandLineArguments(a.arguments(), &flashParams);
    if (ret) {
        cerr << "Failed to parse arguments" << endl;
        exit(1);
    }

    ret = getDeviceNodePath(&flashParams.outputFile);
    if (ret){
        cerr << "Failed to get device node path" << endl;
        exit(1);
    }

    ret = flashDevice(flashParams);

    if (ret) {
        cerr << "Failed to flash device" << endl;
        exit(1);
    }

    a.quit();
}


