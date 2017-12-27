HEADERS += $$PWD/rpiboot.h \
    $$PWD/usb.h \
    $$PWD/usbutils.h
SOURCES += $$PWD/rpiboot.cpp \
    $$PWD/usbutils.cpp

unix {
    LIBUSB_HOME = "libs/libusb/g++64"
    INCLUDEPATH += "$$LIBUSB_HOME/include"
    LIBS += -L"$$LIBUSB_HOME/lib/" -lusb-1.0
}

win32 {
    LIBS += -llibusb-1.0
}

RESOURCES += $$PWD/usbboot_files.qrc

