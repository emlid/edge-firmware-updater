#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>


void list_properties(struct udev_device *dev){
    struct udev_list_entry *list;
    struct udev_list_entry *node;

    list = udev_device_get_properties_list_entry (dev);
    printf("list\n");
    udev_list_entry_foreach (node, list) {
        printf ("%s: %s\n", udev_list_entry_get_name(node), udev_list_entry_get_value(node));
    }
}



static struct udev_device* get_child(struct udev* udev, struct udev_device* parent,
                                     const char* subsystem) {
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
    struct udev_device* scsi_disk;
    struct udev_device* usb;

    enumerate = udev_enumerate_new(udev);


udev_enumerate_add_match_subsystem(enumerate, "scsi");
    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "scsi_device");
//    udev_enumerate_add_match_subsystem(enumerate, "block");
//    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "partition");
    udev_enumerate_scan_devices(enumerate);

    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(entry, devices) {
        path = udev_list_entry_get_name(entry);
        scsi = udev_device_new_from_syspath(udev, path);

        block = get_child(udev, scsi, "block");
        scsi_disk = get_child(udev, scsi, "scsi_disk");

        usb = udev_device_get_parent_with_subsystem_devtype(scsi, "usb", "usb_device");

        if (/*block && scsi_disk && usb*/scsi&&usb) {
            printf("Device Node Path = %s\n", udev_device_get_devnode(block));
            printf("  VID/PID: %s/%s\n",
                   udev_device_get_sysattr_value(usb, "idVendor"),
                   udev_device_get_sysattr_value(usb, "idProduct"));

            // printf("  Vendor: %s\n", udev_device_get_sysattr_value(scsi, "vendor"));

            printf("  Manufacturer:  %s\n", udev_device_get_sysattr_value(usb, "manufacturer"));
            printf("  Product: %s\n", udev_device_get_sysattr_value(usb,"product"));
            printf("  Partition Size: %s\n", udev_device_get_property_value(scsi, "ID_PART_ENTRY_SIZE"));
            printf("  Serial: %s\n", udev_device_get_sysattr_value(usb, "serial"));
            printf("  size?: %s\n", udev_device_get_sysattr_value(block, "size"));
        }


        if (block) {
            udev_device_unref(block);
        }

        if (scsi_disk) {
            udev_device_unref(scsi_disk);
        }

        udev_device_unref(scsi);
    }

    udev_enumerate_unref(enumerate);
}





void udev_device_parameters(struct udev_device *dev) {
    if (dev) {
        printf("Got Device\n");
      //  list_properties(dev);
        printf("   Name: %s\n", udev_device_get_property_value(dev, "ID_MODEL"));
        printf("   sysnameame: %s\n", udev_device_get_sysname(dev));
        printf("   Partition Size: %s\n", udev_device_get_property_value(dev, "ID_PART_ENTRY_SIZE"));
            printf("  size?: %s\n", udev_device_get_sysattr_value(dev, "size"));
        //printf("   Node: %s\n ", udev_device_get_property_value(dev, "DEVPATH"));
        printf("   Node: %s\n", udev_device_get_devnode (dev));
        printf("   Sysname: %s\n", udev_device_get_sysname (dev));
        printf("   Subsystem: %s\n", udev_device_get_subsystem(dev));
        printf("   Devtype: %s\n", udev_device_get_devtype(dev));
        printf("   Action: %s\n", udev_device_get_action(dev));
        udev_device_unref(dev);
    } else {
        printf("No Device from receive_device(). An error occured.\n");
    }

}

int main() {
    struct udev* udev = udev_new();
    struct udev_device *dev;//
    struct udev_device *scsi;//
    struct udev_device* usb;
    struct udev_device* block;
    struct udev_monitor *mon;//
    int fd;//
    struct timeval tv;//
    int ret;//

    mon = udev_monitor_new_from_netlink(udev, "udev");//
    udev_monitor_filter_add_match_subsystem_devtype(mon, "scsi", "scsi_device");//
    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "disk");//
   // udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", NULL);//
    udev_monitor_enable_receiving(mon);//
    fd = udev_monitor_get_fd(mon);//

    enumerate_usb_mass_storage(udev);
    while (1) {
        fd_set fds;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(fd+1, &fds, NULL, NULL, &tv);

        /* Check if our file descriptor has received data. */
        if (ret > 0 && FD_ISSET(fd, &fds)) {
            printf("\nselect() says there should be data\n");

            printf(" +++++++++++ start for device ++++++\n");
            dev = udev_monitor_receive_device(mon);
        //    list_properties(dev);
            udev_device_parameters(dev);
            printf(" +++++++++++ end for device ++++++\n");
        }
        usleep(250*1000);
        printf(".");
        fflush(stdout);
    }

    udev_unref(udev);
    return 0;
}
