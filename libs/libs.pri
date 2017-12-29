DEST_LIBDIR    = $$shell_path($$DESTDIR)

unix {
    # Qt libs
    QT_DEST_LIBDIR = $$DEST_LIBDIR

    QT_LIB_LIST = \
        libQt5Core.so.5 \
        libQt5Network.so.5 \
        libQt5RemoteObjects.so.5 \
        libQt5Test.so.5 \
        libicudata.so.56 \
        libicui18n.so.56 \
        libicuuc.so.56

    for(QT_LIB, QT_LIB_LIST) {
        copy_$${QT_LIB}.target   = $$QT_DEST_LIBDIR/$$QT_LIB
        copy_$${QT_LIB}.commands = \
            @echo Copy $${QT_LIB} \
            && $$QMAKE_COPY --dereference $$[QT_INSTALL_LIBS]/$$QT_LIB $$QT_DEST_LIBDIR \

        QMAKE_EXTRA_TARGETS += copy_$${QT_LIB}
        POST_TARGETDEPS     += $$QT_DEST_LIBDIR/$$QT_LIB
    }

    # libusb

    LIBUSB_LIB_NAME = "libusb-1.0.so"
    LIBUSB_LIB_HOME = "$$PWD/libusb/g++64/lib"
    LIBUSB_LIB_BUILD_TARGET = "$${DEST_LIBDIR}/$${LIBUSB_LIB_NAME}"

    copy_libusb.target   = $$LIBUSB_LIB_BUILD_TARGET
    copy_libusb.commands = \
        @echo Copy $$LIBUSB_LIB_NAME  \
        && $$QMAKE_COPY $$LIBUSB_LIB_HOME/$$LIBUSB_LIB_NAME $$DEST_LIBDIR \

    QMAKE_EXTRA_TARGETS += copy_libusb
    POST_TARGETDEPS     += $$LIBUSB_LIB_BUILD_TARGET
}

win32 {
    # First. Copy libusb
    LIBUSB_LIB_NAME = "libusb-1.0.dll"
    LIBUSB_LIB_HOME = "$$PWD/libusb/msvc2015_32/lib"

    LIBUSB_LIB_BUILD_TARGET = $$shell_path("$${DEST_LIBDIR}/$${LIBUSB_LIB_NAME}")
    copy_libusb.target   = $$LIBUSB_LIB_BUILD_TARGET
    copy_libusb.commands = \
        @echo Copy $$LIBUSB_LIB_NAME  \
        && $$QMAKE_COPY $$shell_path($$LIBUSB_LIB_HOME/$$LIBUSB_LIB_NAME) $$shell_path($$DEST_LIBDIR) \

    QMAKE_EXTRA_TARGETS += copy_libusb
    POST_TARGETDEPS     += $$LIBUSB_LIB_BUILD_TARGET

    # Second. Copy all needed Qt libraries with 'windeployqt' command
    QT_BIN_DIR       = $$dirname(QMAKE_QMAKE)
    DEPLOY_TARGET    = $$shell_quote($$shell_path($$DESTDIR/$${TARGET}.exe))
    QMAKE_POST_LINK += $$QT_BIN_DIR/windeployqt --no-compiler-runtime $${DEPLOY_TARGET}
}
