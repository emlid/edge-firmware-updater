unix {
    LIBS += \
        -ludev \
        -lblkid

    SOURCES += \
        $$PWD/linux_StorageDeviceInfo.cpp \
        $$PWD/linux_Mountpoint.cc \
        $$PWD/linux_Partition.cc \
        $$PWD/linux_StorageDeviceFile.cc \

} else:win32 {
    LIBS += \
        -lsetupAPI

    SOURCES += \
        $$PWD/win_StorageDeviceInfo.cc \
        $$PWD/win_Mountpoint.cc \
        $$PWD/win_StorageDeviceFile.cc \
        $$PWD/win_Partition.cpp
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
    $$PWD/error.cc \

