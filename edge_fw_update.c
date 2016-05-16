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

void free_parameters(struct scsi_block_device dev_prop){
    free (dev_prop.size);
    free (dev_prop.node_path);
    free (dev_prop.vid);
    free (dev_prop.pid);
    free (dev_prop.manufacturer);
    free (dev_prop.product);
}

void print_scsi_block(struct scsi_block_device* dev) {
    printf("   VID/PID: %s/%s\n", dev->vid, dev->pid);
    printf("   Manufacturer: %s\n", dev->manufacturer);
    printf("   Product: %s\n", dev->product);
    printf("   Size: %s blocks (%f GB)\n", dev->size, atof(dev->size) * 512 / 1e9);
    printf("   Node path: %s\n\n", dev->node_path);
}


char* get_field_value(struct udev_device * dev, char * property_name){
    char * field;
    field = (char*) malloc(strlen((char*) udev_device_get_property_value(dev, property_name)));
    strcpy(field, (char*) udev_device_get_property_value(dev, property_name));
    return field;
}

void fill_device_param_list (struct udev_device* dev, struct scsi_block_device* dist) {
    const char* subsys;
    subsys = udev_device_get_subsystem(dev);
    if (strcmp (subsys,"scsi") == 0) {
        dist->vid = get_field_value(dev, "ID_VENDOR_ID");
        dist->pid = get_field_value(dev, "ID_MODEL_ID");
        dist->manufacturer = get_field_value(dev, "ID_VENDOR");
        dist->product = get_field_value(dev, "ID_MODEL");
        dist->scsi = 1;
    }
    if (strcmp (subsys,"block") == 0) {
        dist->size = (char*) malloc(strlen((char*) udev_device_get_sysattr_value(dev, "size")));
        strcpy( dist->size, (char*) udev_device_get_sysattr_value(dev, "size"));
        
        dist->node_path = (char*) malloc(strlen((char*) udev_device_get_devnode(dev)));
        strcpy(dist->node_path, (char*) udev_device_get_devnode(dev));
        dist->block = 1;
        
    }
}



void run_rpiboot() {
    int ret =  system("sudo rpiboot");
    if (WEXITSTATUS(ret) < 0) {
        fprintf(stderr, "Rpiboot failed\n");
        exit(EXIT_FAILURE); 
    }
}



int main() {
    struct udev* udev = udev_new();
    struct udev_device* dev;
    struct udev_monitor* mon;
    struct scsi_block_device dev_properties;
    int fd;
    struct timeval tv;
    int ret;


    dev_properties.scsi = 0;
    dev_properties.block = 0;

    run_rpiboot();

    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "scsi", "scsi_device");
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
            if (!strcmp( udev_device_get_action(dev), "add") && !strcmp( udev_device_get_property_value(dev, "ID_VENDOR_ID"), "0a5c")){
                fill_device_param_list(dev, &dev_properties);
                if ((dev_properties.block == 1) && (dev_properties.scsi == 1)) {
                    printf("\nDetected storage device:\n");
                    print_scsi_block(&dev_properties);
                    dev_properties.block = 0;
                    dev_properties.scsi = 0;
                    break;
                }
            }
        }
    }

    udev_device_unref(dev);
    udev_unref(udev);
    free_parameters(dev_properties);    
    return 0;
}
