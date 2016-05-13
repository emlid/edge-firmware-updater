#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

struct scsi_block_device {
    char* vid;
    char* pid;
    char* manufacturer;
    char* product;
    char* node_path;
    char* size;
    int scsi;
    int block;
};


void print_scsi_block(struct scsi_block_device* dev) {
    printf("   VID/PID: %s/%s\n", dev->vid, dev->pid);
    printf("   Manufacturer: %s\n", dev->manufacturer);
    printf("   Product: %s\n", dev->product);
    printf("   Size: %s blocks\n", dev->size);
    printf("   Node path: %s\n", dev->node_path);
}


void fill_device_param_list (struct udev_device* dev, struct scsi_block_device* dist) {
    const char* subsys;
    subsys = udev_device_get_subsystem(dev);
    if (strcmp (subsys,"scsi") == 0) {
        dist->vid = (char*) udev_device_get_property_value(dev, "ID_VENDOR_ID");
        dist->pid = (char*) udev_device_get_property_value(dev, "ID_MODEL_ID");
        dist->manufacturer = (char*)udev_device_get_property_value(dev, "ID_VENDOR");
        dist->product = (char*)udev_device_get_property_value(dev, "ID_MODEL");
        dist->scsi = 1;
    }
    if (strcmp (subsys,"block") == 0) {
        dist->size = (char*)udev_device_get_sysattr_value(dev, "size");
        dist->node_path = (char*)udev_device_get_devnode(dev);
        dist->block = 1;
    }
    if ((dist->block == 1) && (dist->scsi == 1)) {
        printf("\nAction: %s\n", udev_device_get_action(dev));
        print_scsi_block(dist);
        dist->block = 0;
        dist->scsi = 0;
        udev_device_unref(dev);
    }
}

void list_properties(struct udev_device *dev) {
    struct udev_list_entry *list;
    struct udev_list_entry *node;

    list = udev_device_get_properties_list_entry (dev);
    printf("list\n");
    udev_list_entry_foreach (node, list) {
        printf ("%s: %s\n", udev_list_entry_get_name(node), udev_list_entry_get_value(node));
    }
}



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
        break;
    }

    udev_enumerate_unref(enumerate);
    return child;
}

static void enumerate_usb_mass_storage(struct udev* udev) {
    struct udev_enumerate* enumerate;
    struct udev_list_entry *devices;
    struct udev_list_entry *entry;
    const char* path;
    struct udev_device* scsi;
    struct udev_device* block;
    struct udev_device* usb;

    enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(enumerate, "scsi");
    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "scsi_device");

    udev_enumerate_scan_devices(enumerate);

    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(entry, devices) {
        path = udev_list_entry_get_name(entry);
        scsi = udev_device_new_from_syspath(udev, path);

        block = get_child(udev, scsi, "block");

        usb = udev_device_get_parent_with_subsystem_devtype(scsi, "usb", "usb_device");

        if (block && usb) {
            printf("Device Node Path = %s\n", udev_device_get_devnode(block));
            printf("  VID/PID: %s/%s\n",
                    udev_device_get_property_value(usb, "ID_VENDOR_ID"),
                    udev_device_get_property_value(usb, "ID_MODEL_ID"));
            printf("  Manufacturer:  %s\n", udev_device_get_property_value(usb, "ID_VENDOR"));
            printf("  Product: %s\n", udev_device_get_property_value(usb,"ID_MODEL"));
            printf("  Size: %s blocks\n", udev_device_get_sysattr_value(block, "size"));
        }

        if (block) {
            udev_device_unref(block);
        }

        udev_device_unref(scsi);
    }

    udev_enumerate_unref(enumerate);
}






int main() {
    struct udev* udev = udev_new();
    struct udev_device* dev;
    struct udev_device* scsi;
    struct udev_device* usb;
    struct udev_device* block;
    struct udev_monitor* mon;
    struct scsi_block_device dev_properties;
    int fd;
    struct timeval tv;
    int ret;

    dev_properties.scsi = 0;
    dev_properties.block = 0;

    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "scsi", "scsi_device");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "disk");
    udev_monitor_enable_receiving(mon);
    fd = udev_monitor_get_fd(mon);

    enumerate_usb_mass_storage(udev);

    while (1) {
        fd_set fds;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(fd+1, &fds, NULL, NULL, &tv);

        if (ret > 0 && FD_ISSET(fd, &fds)) {
            dev = udev_monitor_receive_device(mon);
            fill_device_param_list(dev, &dev_properties);
        }
        usleep(500*1000);
        printf(".");
        fflush(stdout);
    }

    udev_unref(udev);
    return 0;
}
