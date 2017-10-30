HEADERS += $$PWD/LinuxStorageDevice.h \
           $$PWD/LinuxStorageDeviceManager.h

SOURCES += $$PWD/LinuxStorageDevice.cpp \
           $$PWD/LinuxStorageDeviceManager.cpp

LIBS += -ludev \   # For scanning devices
        -L/usr/lib
