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
}

win32 {
    # First. Copy libusb
    WIN_LIBRARY_ROOT_PATH = "C:"
    LIBUSB_PATH =  $$shell_path("$${WIN_LIBRARY_ROOT_PATH}/libusb")

    QMAKE_POST_LINK += $$escape_expand(\\n) $$QMAKE_COPY \
        \"$$shell_path($${LIBUSB_PATH}/MS32/dll/libusb-1.0.dll)\" \
        \"$$shell_path($$DESTDIR)\"

    # Second. Copy all needed Qt libraries with 'windeployqt' command
    QT_BIN_DIR       = $$dirname(QMAKE_QMAKE)
    DEPLOY_TARGET    = $$shell_quote($$shell_path($$DESTDIR/$${TARGET}.exe))
    QMAKE_POST_LINK += && "$$QT_BIN_DIR/windeployqt" --no-compiler-runtime $${DEPLOY_TARGET}
}
