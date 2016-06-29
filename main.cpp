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

/*static const uint16_t VENDOR_ID  = 0xa5c;
static const uint16_t VENDOR_ID_DEC = 2652;
static const uint16_t PRODUCT_ID1 = 0x2763;
static const uint16_t PRODUCT_ID2 = 0x2764;
*/
#define VENDOR_ID       0xa5c
#define VENDOR_ID_CHAR   "0a5c"
#define PRODUCT_ID1     0x2763
#define PRODUCT_ID2     0x2764



//HARDCODE DETECTED
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
        if (strcmp(udev_device_get_property_value(child, "ID_VENDOR_ID"), VENDOR_ID_CHAR) == 0)  //need to get rid of hardcode
            break;
    }

    udev_enumerate_unref(enumerate);
    return child;
}






static void findMountedDeviceByVid(struct udev* udev, char* devnode, const char* vid) {
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
            printf("Found device:\n");
            printf(" VID/PID: %s/%s\n",
                    udev_device_get_property_value(block, "ID_VENDOR_ID"),
                    udev_device_get_property_value(block, "ID_MODEL_ID"));
            printf(" Node Path = %s\n", udev_device_get_devnode(block));

            if (strcmp(udev_device_get_property_value(block, "ID_VENDOR_ID"), vid) == 0) {
                strcpy(devnode, (char*)udev_device_get_devnode(block));              
                udev_device_unref(scsi);
                break;
            }
            udev_device_unref(block);
        }
        udev_device_unref(scsi);
    }
    udev_enumerate_unref(enumerate);
}




void startRpiBoot(QThread *thread){
    thread->setObjectName("rpiboot Thread");
    QObject::connect(thread, &QThread::started, rpiboot);
    thread->start();
}


void catchNewDeviceByVid(char* devnode, const char* vid){

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
                printf("\nDetected storage device:\n");
                printf("   Node: %s\n", udev_device_get_devnode(dev));
                printf("   Subsystem: %s\n", udev_device_get_subsystem(dev));
                printf("   Devtype: %s\n", udev_device_get_devtype(dev));
                strcpy(devnode, (char*)udev_device_get_devnode(dev));
                break;

            }
        }
    }

    udev_device_unref(dev);
    udev_unref(udev);
}



FlashingParameters::FlashingParameters(){
    strcpy(_blockSize, "bs=1000000");
    strcpy(_inputFile, "if=");
    strcpy(_outputFile, "of=");
}

void FlashingParameters::setBs(char *bs) {
    strcpy(_blockSize, bs);
}

void FlashingParameters::setIf(char *pathToFile) {
    strcat(_inputFile, pathToFile);
}

void FlashingParameters::setOf(char *pathToDevice) {
    strcat(_outputFile, pathToDevice);
}

char * FlashingParameters::getBs(){
    return _blockSize;
}

char * FlashingParameters::getOf(){
    return _outputFile;
}

char * FlashingParameters::getIf(){
    return _inputFile;
}

void FlashingParameters::getAllParams(char *params[]){
    params[0] = getBs();
    params[1] = getIf();
    params[2] = getOf();
}


void checkArguments(int argc, char **argv) {

    FILE *fp1;

    if (argc < 2) {
        fprintf(stderr, "need 1 argument (path to image)\n");
        QThread::sleep(3);
        exit(1);
    }

    if(strcmp(*(argv+1) + strlen(*(argv+1)) - 4, ".img"))
    {
        printf("File needs to be .img format\n");
        exit(-1);
    }


    fp1 = fopen(*(argv+1), "r");
    if (fp1 == NULL)
    {
        printf("Cannot open file %s\n", *(argv+1));
        exit(-1);
    }
    fclose(fp1);
}


int main(int argc, char *argv[])
{
    FlashingParameters flashParams;

    struct udev *udev = udev_new();
    libusb_context *ctx;

    char founded[100];

    QCoreApplication a(argc, argv);

    checkArguments(argc, argv);

    int ret = libusb_init(&ctx);
    if (ret) {
        printf("Failed to initialise libUSB\n");
        exit(-1);
    }

    flashParams.setIf(*(argv+1));

    // Get all devices connected to system
    libusb_device **devs;

    if (libusb_get_device_list(NULL, &devs) < 0) {
        qDebug() << "Failed to get devices list";
        return 0;
    }

    // Scan for Device VID & PID
    libusb_device *dev;

    int i = 0;

    while ( (dev = devs[i++]) != NULL ) {
        struct libusb_device_descriptor desc;

        if (libusb_get_device_descriptor(dev, &desc) < 0) {
            qDebug() << "can't get device descriptor";
            continue;
        }

        if (desc.idVendor == VENDOR_ID) {
            printf("Detected:\n");
            printf(" VENDOR_ID: %x (%d)\n" , desc.idVendor, desc.idVendor);
            printf(" PRODUCT_ID: %x (%d)\n" , desc.idProduct, desc.idProduct);


            if (desc.idProduct == PRODUCT_ID1 || desc.idProduct == PRODUCT_ID2) {
                printf("\n    configuing udev_monitor and starting rpiboot...\n\n");

                QThread rpibootThread;
                startRpiBoot(&rpibootThread);

                catchNewDeviceByVid(founded, VENDOR_ID_CHAR);   
                flashParams.setOf(founded);

            } else {
                printf("\nenumerating...\n\n");

                findMountedDeviceByVid(udev, founded, VENDOR_ID_CHAR);
                flashParams.setOf(founded);
            }
            if(strlen(founded) == 0) {
                printf("empty device address\n");
                return -1;
            }
            printf("\nflashing CM:\n");
            printf("%s\n", flashParams.getBs());
            printf("%s\n", flashParams.getIf());
            printf("%s\n", flashParams.getOf());
            flashDevice(flashParams);

            break;
        }
    }


    libusb_exit(ctx);

    return a.exec();
}


