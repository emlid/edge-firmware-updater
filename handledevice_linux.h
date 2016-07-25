#ifndef HANDLEDEVICE_LINUX_H
#define HANDLEDEVICE_LINUX_H

#include <QString>


int getDeviceNodePath(QString *);
void catchNewDeviceByVid(QString*, const char*);
void findMountedDeviceByVid(QString*, const char*);
struct udev_device* get_child(struct udev*, struct udev_device*, const char*);
#endif // HANDLEDEVICE_LINUX_H
