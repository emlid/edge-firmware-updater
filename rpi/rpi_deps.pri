linux {
    LIBS += -lusb-1.0
}

macx {
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib -lusb-1.0
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
