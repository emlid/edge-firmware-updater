HEADERS += $$PWD/rpiboot.h \
    $$PWD/usb.h \
    $$PWD/usbutils.h
SOURCES += $$PWD/rpiboot.cpp \
    $$PWD/usbutils.cpp

unix {
    LIBS += -lusb-1.0
}

win32 {
    LIBS += -llibusb-1.0
}

RESOURCES += $$PWD/usbboot_files.qrc

