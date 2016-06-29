

#include "libusb-1.0/libusb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpiboot.h>
#include <unistd.h>
#include <QThread>
#include <QtDebug>

int verbose = 0;
int out_ep = 1;
int in_ep = 2;


libusb_device_handle * LIBUSB_CALL open_device_with_vid(
	libusb_context *ctx, uint16_t vendor_id)
{
	struct libusb_device **devs;
	struct libusb_device *found = NULL;
	struct libusb_device *dev;
	struct libusb_device_handle *handle = NULL;
	size_t i = 0;
	int r;

	if (libusb_get_device_list(ctx, &devs) < 0)
		return NULL;

	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0)
			goto out;
		if (desc.idVendor == vendor_id) {
			if(desc.idProduct == 0x2763 ||
			   desc.idProduct == 0x2764)
			{
				found = dev;
				break;
			}
		}
	}

	if (found) {
		r = libusb_open(found, &handle);
		if (r < 0)
		{
			if(verbose) printf("Failed to open the requested device\n");
			handle = NULL;
		}
	}

out:
	libusb_free_device_list(devs, 1);
	return handle;

}

int Initialize_Device(libusb_context ** ctx, libusb_device_handle ** usb_device)
{
	int ret = 0;
	int interface;
	struct libusb_config_descriptor *config;

	*usb_device = open_device_with_vid(*ctx, 0x0a5c);
	if (*usb_device == NULL)
	{
		return -1;
	}

	libusb_get_active_config_descriptor(libusb_get_device(*usb_device), &config);

	if(config->bNumInterfaces == 1)
	{
		interface = 0;
		out_ep = 1;
		in_ep = 2;
	}
	else
	{
		interface = 1;
		out_ep = 3;
		in_ep = 4;
	}

	ret = libusb_claim_interface(*usb_device, interface);
	if (ret)
	{
		printf("Failed to claim interface\n");
		return ret;
	}

	printf("Initialised device correctly\n");

	return ret;
}

int ep_write(unsigned char *buf, int len, libusb_device_handle * usb_device)
{
	int a_len;
	int ret =
	    libusb_control_transfer(usb_device, LIBUSB_REQUEST_TYPE_VENDOR, 0,
				    len & 0xffff, len >> 16, NULL, 0, 1000);

	if(ret != 0)
	{
		printf("Failed control transfer\n");
		return ret;
	}

	ret = libusb_bulk_transfer(usb_device, out_ep, buf, len, &a_len, 100000);

	return a_len;
}

int ep_read(unsigned char *buf, int len, libusb_device_handle * usb_device)
{
    libusb_control_transfer(usb_device,
				    LIBUSB_REQUEST_TYPE_VENDOR |
				    LIBUSB_ENDPOINT_IN, 0, len & 0xffff,
				    len >> 16, buf, len, 1000);


    return len;
}

void rpiboot(void)
{

	int result;
	libusb_context *ctx;
	libusb_device_handle *usb_device;
	unsigned char *txbuf;
	int size;
    int retcode = 0;
	int last_serial = -1;
	FILE *fp1, *fp2, *fp;

    char def1_loc[] = "../usbboot_files/usbbootcode.bin";
    char def2_loc[] = "../usbboot_files/msd.elf";

	char *def1, *def2;

	char *stage1   = NULL, *stage2     = NULL;

    def1 = def1_loc;
    def2 = def2_loc; 

    stage1   = def1;
	stage2   = def2;

	struct MESSAGE_S {
		int length;
		unsigned char signature[20];
	} message;
	
#if defined (__CYGWIN__)
  //printf("Running under Cygwin\n");
#else
	//exit if not run as sudo
	if(getuid() != 0)
	{
		printf("Must be run with sudo...\n");
		exit(-1);
	}
#endif

	fp1 = fopen(stage1, "rb");
	if (fp1 == NULL)
	{
		printf("Cannot open file %s\n", stage1);

		exit(-1);
	}

	fp2 = fopen(stage2, "rb");
	if (fp2 == NULL)
	{
		printf("Cannot open file %s\n", stage2);
		exit(-1);
	}
	if(strcmp(stage2 + strlen(stage2) - 4, ".elf"))
	{
		printf("Third stage needs to be .elf format\n");
		exit(-1);
	}

	int ret = libusb_init(&ctx);
	if (ret)
	{
		printf("Failed to initialise libUSB\n");
		exit(-1);
	}

	libusb_set_debug(ctx, verbose ? LIBUSB_LOG_LEVEL_WARNING : 0);

	do
	{
		struct libusb_device_descriptor desc;
		struct libusb_config_descriptor *config;

		printf("Waiting for BCM2835 ...\n");

		// Wait for a device to get plugged in
		do
		{
			result = Initialize_Device(&ctx, &usb_device);
			if(result == 0)
			{
				libusb_get_device_descriptor(libusb_get_device
								 (usb_device), &desc);
				printf("Found serial number %d\n", desc.iSerialNumber);
				// Make sure we've re-enumerated since the last time
				if(desc.iSerialNumber == last_serial)
				{
					result = -1;
					libusb_close(usb_device);
				}			

				libusb_get_active_config_descriptor(libusb_get_device(usb_device), &config);
			}

			if (result)
			{
				usleep(100);
			}
		}
		while (result);

		last_serial = desc.iSerialNumber;
		printf("Found serial = %d: writing file %s\n",
		       desc.iSerialNumber,
		       desc.iSerialNumber == 0 ? stage1 : stage2);
		fp = desc.iSerialNumber == 0 ? fp1 : fp2;

		fseek(fp, 0, SEEK_END);
		message.length = ftell(fp);
		fseek(fp, 0, SEEK_SET);



		txbuf = (unsigned char *)malloc(message.length);
		if (txbuf == NULL)
		{
			printf("Failed to allocate memory\n");
			exit(-1);
		}

		size = fread(txbuf, 1, message.length, fp);

		size = ep_write((unsigned char *)&message, sizeof(message), usb_device);
		if (size != sizeof(message))
		{
			printf("Failed to write correct length, returned %d\n",
			       size);
			exit(-1);
		}
		size = ep_write(txbuf, message.length, usb_device);
		if (size != message.length)
		{
			printf("Failed to read correct length, returned %d\n",
			       size);
			exit(-1);
		}

		sleep(1);

		size = ep_read((unsigned char *)&retcode, sizeof(retcode), usb_device);

		if (retcode == 0)
		{
			printf("Successful read %d bytes \n", size);

		}
		else
			printf("Failed : 0x%x", retcode);

		libusb_close(usb_device);
		sleep(1);
        free(txbuf);
	}
	while(fp == fp1);

	libusb_exit(ctx);
    QThread::currentThread()->exit(0);
}
