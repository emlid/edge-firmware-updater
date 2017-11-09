HEADERS += $$PWD/rpiboot.h
SOURCES += $$PWD/rpiboot.cpp

unix {
    LIBS += -lusb-1.0
}

win32 {
    LIBS += -llibusb-1.0
}

RESOURCES += $$PWD/usbboot_files.qrc

