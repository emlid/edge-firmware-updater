unix {
    LIBS += \
        -ludev \
        -lblkid

    SOURCES += \
        $$PWD/linux_StorageDeviceInfo.cpp \
        $$PWD/linux_Mountpoint.cc \
        $$PWD/linux_Partition.cc \
        $$PWD/linux_StorageDeviceFile.cc \
}

HEADERS += \
    $$PWD/devlib.h \
    $$PWD/Mountpoint.h \
    $$PWD/Partition.h \
    $$PWD/StorageDeviceInfo.h \
    $$PWD/StorageDeviceFile.h \
    $$PWD/util.h \
    $$PWD/error.h

SOURCES += \
    $$PWD/error.cc
