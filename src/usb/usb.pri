HEADERS += \
    $$PWD/rpiboot.h \
    $$PWD/usb.h \
    $$PWD/usbutils.h \

SOURCES += \
    $$PWD/rpiboot.cpp \
    $$PWD/usbutils.cpp \

unix {
    LIBS += -lusb-1.0
}

win32 {
    WIN_LIBRARY_ROOT_PATH = "C:"

    LIBUSB_PATH =  $$shell_path("$${WIN_LIBRARY_ROOT_PATH}/libusb")
    !exists("$$LIBUSB_PATH") {
        error("libusb not found. Check README.md for installation steps")
    } else {
        INCLUDEPATH += $$shell_path("$${LIBUSB_PATH}/include")
        LIBS +=      -L$$shell_path("$${LIBUSB_PATH}/MS32/dll/") -llibusb-1.0
    }
}

RESOURCES += $$PWD/usbboot_files.qrc

