#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QtCore>

#include "RpiBootImpl.h"

#if defined(Q_OS_WIN)
#pragma warning(push)
#pragma warning(disable:4200)
#include <libusb-1.0/libusb.h>
#pragma warning(pop)
#else
#include <libusb-1.0/libusb.h>
#endif



typedef struct MESSAGE_S
{
        int length;
        unsigned char signature[20];
} boot_message_t;


class RpiBootPrivate
{
public:
    RpiBootPrivate(int vid, QList<int> const& pids)
        : _vid(vid),
          _pids(pids),
          _signed_boot(0),
          _verbose(1),
          _loop(0),
          _directory(":/usbboot_files/") { }

    int vid(void) const { return _vid; }

    QList<int> const& pids(void) const { return _pids; }

    libusb_device_handle* LIBUSB_CALL open_device_with_vid(libusb_context *ctx, uint16_t vendor_id);

    int initialize_device(libusb_context ** ctx, libusb_device_handle ** usb_device);

    int ep_write(void *buf, int len, libusb_device_handle * usb_device);

    int ep_read(void *buf, int len, libusb_device_handle * usb_device);

    int second_stage_prep(QFile& fp, QFile& fp_sig);

    int second_stage_boot(libusb_device_handle *usb_device);

    int file_server(libusb_device_handle * usb_device);

    int boot(void);

private:
    int _vid;
    QList<int> _pids;

    int _signed_boot;
    int _verbose;
    int _loop;
    QString _directory;
    int _out_ep;
    int _in_ep;

    void *second_stage_txbuf;
    boot_message_t boot_message;
};


static void initResources(void) {
    Q_INIT_RESOURCE(usbboot_files);
}

usb::RpiBootImpl::RpiBootImpl(int vid, QList<int> const& pids)
    : _pimpl(new RpiBootPrivate(vid, pids))
{
    ::initResources();
}

usb::RpiBootImpl::~RpiBootImpl(void) { }


int usb::RpiBootImpl::bootAsMassStorage_core(void)
{
    return _pimpl->boot();
}

/* RpiBootPrivate implementation */

libusb_device_handle * LIBUSB_CALL RpiBootPrivate
    ::open_device_with_vid(libusb_context *ctx, uint16_t vendor_id)
{
    struct libusb_device **devs;
    struct libusb_device *found = NULL;
    struct libusb_device *dev;
    struct libusb_device_handle *handle = NULL;
    uint32_t i = 0;
    int r;

    if (libusb_get_device_list(ctx, &devs) < 0)
        return NULL;

    while ((dev = devs[i++]) != NULL) {
        struct libusb_device_descriptor desc;
        r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0)
            goto out;
        if(_verbose) {
            qInfo() << QString()
                .asprintf("Found device %u idVendor=0x%04x idProduct=0x%04x", i, desc.idVendor, desc.idProduct);
        }

        if (desc.idVendor == vendor_id) {
            if(_pids.contains(desc.idProduct))
            {
                if(_verbose) {
                    qInfo() << "Device located successfully";
                }
                found = dev;
                break;
            }
        }
    }

    if (found) {
        QThread::sleep(1);
        r = libusb_open(found, &handle);
        if (r < 0)
        {
            if(_verbose) {
                qInfo("Failed to open the requested device");
            }
            handle = NULL;
        }
    }

out:
    libusb_free_device_list(devs, 1);
    return handle;

}


int RpiBootPrivate::initialize_device(libusb_context ** ctx, libusb_device_handle ** usb_device)
{
    int ret = 0;
    int interface;
    struct libusb_config_descriptor *config;

    *usb_device = open_device_with_vid(*ctx, _vid);
    if (*usb_device == NULL)
    {
        QThread::usleep(200);
        return -1;
    }

    libusb_get_active_config_descriptor(libusb_get_device(*usb_device), &config);
    if(config == NULL)
    {
        qFatal("Failed to read config descriptor");
        QThread::currentThread()->exit(-1);
    }

    // Handle 2837 where it can start with two interfaces, the first is mass storage
    // the second is the vendor interface for programming
    if(config->bNumInterfaces == 1) {
        interface = 0;
        _out_ep = 1;
        _in_ep = 2;
    } else {
        interface = 1;
        _out_ep = 3;
        _in_ep = 4;
    }

    ret = libusb_claim_interface(*usb_device, interface);
    if (ret) {
        libusb_close(*usb_device);
        qFatal("Failed to claim interface");
        return ret;
    }

    if(_verbose) {
        qInfo("Initialised device correctly");
    }

    return ret;
}


int RpiBootPrivate::ep_write(void *buf, int len, libusb_device_handle * usb_device)
{
    int a_len = 0;
    int ret =
        libusb_control_transfer(usb_device, LIBUSB_REQUEST_TYPE_VENDOR, 0,
                    len & 0xffff, len >> 16, NULL, 0, 1000);

    if(ret != 0)
    {
        qCritical("Failed control transfer");
        return ret;
    }

    if(len > 0)
    {
        ret = libusb_bulk_transfer(usb_device, _out_ep,(unsigned char*) buf, len, &a_len, 100000);
        if(_verbose) {
            qInfo() << "libusb_bulk_transfer returned " << ret;
        }
    }

    return a_len;
}


int RpiBootPrivate::ep_read(void *buf, int len, libusb_device_handle * usb_device)
{
    int ret =
        libusb_control_transfer(usb_device,
                    LIBUSB_REQUEST_TYPE_VENDOR |
                    LIBUSB_ENDPOINT_IN, 0, len & 0xffff,
                    len >> 16,(unsigned char*) buf, len, 2000);
    if(ret >= 0)
        return len;
    else
        return ret;
}


int RpiBootPrivate::second_stage_prep(QFile& fp, QFile& fp_sig)
{
    boot_message.length = fp.size();

    if(fp_sig.isOpen()) {
        fp_sig.read((char*)boot_message.signature, sizeof(boot_message.signature));
    }

    second_stage_txbuf = (uint8_t *) malloc(boot_message.length);

    if (second_stage_txbuf == NULL) {
        printf("Failed to allocate memory\n");
        return -1;
    }

    auto size = fp.read((char*)second_stage_txbuf, boot_message.length);

    if (size != boot_message.length) {
        printf("Failed to read second stage\n");
        return -1;
    }

    return 0;
}


int RpiBootPrivate::second_stage_boot(libusb_device_handle *usb_device)
{
    int size, retcode = 0;

    size = ep_write(&boot_message, sizeof(boot_message), usb_device);
    if (size != sizeof(boot_message)) {
        printf("Failed to write correct length, returned %d\n", size);
        return -1;
    }

    if(_verbose) {
        qInfo() << "Writing " << boot_message.length << " bytes.";
    }

    size = ep_write(second_stage_txbuf, boot_message.length, usb_device);

    if (size != boot_message.length) {
        qCritical() << "Failed to read correct length, returned " << size;
        return -1;
    }

    QThread::sleep(1);
    size = ep_read((unsigned char *)&retcode, sizeof(retcode), usb_device);

    if (size > 0 && retcode == 0) {
        qInfo() << "Successful read " <<  size << " bytes";
    } else {
        qCritical() << QString().asprintf("Failed : 0x%x", retcode);
    }

    return retcode;
}


int RpiBootPrivate::file_server(libusb_device_handle * usb_device)
{
    int going = 1;
    struct file_message {
        int command;
        char fname[256];
    } message;

    static QFile fp;

    while(going) {
        char message_name[][20] = {"GetFileSize", "ReadFile", "Done"};
        int i = ep_read(&message, sizeof(message), usb_device);
        if(i < 0) {
            // Drop out if the device goes away
            if(i == LIBUSB_ERROR_NO_DEVICE || i == LIBUSB_ERROR_IO)
                break;
            QThread::sleep(1);
            continue;
        }

        if (_verbose) {
            qInfo() << "Received message" << message_name[message.command] << ':'<< message.fname;
        }

        // Done can also just be null filename
        if(strlen(message.fname) == 0) {
            ep_write(NULL, 0, usb_device);
            break;
        }

        switch(message.command)
        {
            case 0: // Get file size
                if (fp.isOpen()) {
                    fp.close();
                }

                qInfo() << "Request: reading file " << message.fname;

                fp.setFileName(_directory + QString(message.fname));

                fp.open(QIODevice::ReadOnly);

                if(strlen(message.fname) && fp.isOpen()) {
                    int file_size = fp.size();

                    if(_verbose) {
                        qInfo() << "File size = " << file_size << "bytes";
                    }

                    int sz = libusb_control_transfer(usb_device, LIBUSB_REQUEST_TYPE_VENDOR, 0,
                        file_size & 0xffff, file_size >> 16, NULL, 0, 1000);

                    if(sz < 0) {
                        return -1;
                    }
                } else {
                    ep_write(NULL, 0, usb_device);
                    if(_verbose) {
                        qInfo() << "Cannot open file " << message.fname;
                    }
                    break;
                }
                break;

            case 1: // Read file
                if(fp.isOpen())
                {
                    int file_size;
                    void *buf;

                    qInfo() << "Sending: " << message.fname;

                    file_size = fp.size();

                    buf = malloc(file_size);
                    if(buf == NULL) {
                        qFatal("Failed to allocate buffer for file");
                        return -1;
                    }

                    int read = fp.read((char*)buf, file_size);

                    if(read != file_size) {
                        qCritical("Failed to read from input file");
                        free(buf);
                        return -1;
                    }

                    int sz = ep_write(buf, file_size, usb_device);

                    free(buf);
                    fp.close();

                    if(sz != file_size) {
                        qCritical("Failed to write complete file to USB device");
                        return -1;
                    }
                } else {
                    if(_verbose) {
                        qCritical() << "No file " << message.fname << " found.";
                    }
                    ep_write(NULL, 0, usb_device);
                }
                break;

            case 2: // Done, exit file server
                going = 0;
                break;

            default:
                qInfo("Unknown message");
                return -1;
        }
    }

    qInfo() << "Second stage boot server done";
    return 0;
}


int RpiBootPrivate::boot(void)
{
    libusb_context *ctx;
    libusb_device_handle *usb_device;
    struct libusb_device_descriptor desc;
    struct libusb_config_descriptor *config;

    // flush immediately
    setbuf(stdout, NULL);

    // Default to standard msd directory

    QString bootcodePath(_directory + QString("bootcode.bin"));
    QFile second_stage(bootcodePath);

    if(!second_stage.open(QIODevice::ReadOnly)) {
        qCritical() << "Unable to open 'bootcode.bin' from resources.";
        return -1;
    }

    QFile fp_sign;
    if(_signed_boot) {
        QString bootsigPath(_directory + QString("/bootsig.bin"));
        fp_sign.setFileName(bootsigPath);

        if (!fp_sign.open(QIODevice::ReadOnly)) {
            qCritical() << "Unable to open 'bootsig.bin'\n";
            return -1;
        }
    }

    if (second_stage_prep(second_stage, fp_sign) != 0) {
        qCritical() << "Failed to prepare the second stage bootcode";
        QThread::currentThread()->exit(-1);
    }

    int ret = libusb_init(&ctx);

    if (ret) {
        qFatal("Failed to initialise libUSB");
        QThread::currentThread()->exit(-1);
    }

    libusb_set_debug(ctx, _verbose ? LIBUSB_LOG_LEVEL_WARNING : 0);

    do {
        int last_serial = -1;
        auto const maxPollingTime = 5000;
        auto const sleepTime  = 10;
        auto totalPollingTime = 0;

        qInfo() << "Waiting for BCM2835/6/7";

        // Wait for a device to get plugged in
        do {
            ret = initialize_device(&ctx, &usb_device);
            if(ret == 0) {
                libusb_get_device_descriptor(libusb_get_device(usb_device), &desc);

                if(_verbose) {
                    qInfo() << "Found serial number " << desc.iSerialNumber;
                }

                // Make sure we've re-enumerated since the last time
                if(desc.iSerialNumber == last_serial) {
                    ret = -1;
                    libusb_close(usb_device);
                }

                libusb_get_active_config_descriptor(libusb_get_device(usb_device), &config);
            }

            if (ret) {
                if (totalPollingTime >= maxPollingTime) {
                    qWarning() << "rpiboot timed out";
                    return -1;
                } else {
                    QThread::msleep(sleepTime);
                    totalPollingTime += sleepTime;
                }
            }
        }
        while (ret);

        last_serial = desc.iSerialNumber;

        if(desc.iSerialNumber == 0) {
            qInfo() << "Sending bootcode.bin";
            second_stage_boot(usb_device);
        } else {
            qInfo() << "Second stage boot server";
            file_server(usb_device);
        }

        libusb_close(usb_device);
        QThread::sleep(1);

    } while(_loop || desc.iSerialNumber == 0);

    libusb_exit(ctx);

    qInfo() << "Rpi boot successfully finished.";

    return 0;
}
