HEADERS += linux/LinuxStorageDevice.h \
           linux/LinuxStorageDeviceManager.h

SOURCES += linux/LinuxStorageDevice.cpp \
           linux/LinuxStorageDeviceManager.cpp

LIBS += -ludev \   # For scanning devices
        -L/usr/lib
