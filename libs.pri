unix {
    # copy all needed Qt libs

    QT_LIB_LIST = \
        libQt5Core.so.5 \
        libQt5Network.so.5 \
        libQt5RemoteObjects.so.5 \
        libQt5Test.so.5 \
        libicudata.so.56 \
        libicui18n.so.56 \
        libicuuc.so.56

    DEST_LIBDIR    = $$DESTDIR/lib
    QT_DEST_LIBDIR = $$DEST_LIBDIR/Qt

    QMAKE_POST_LINK += @echo "Copy libraries."
    QMAKE_POST_LINK += && mkdir -p $$QT_DEST_LIBDIR

    for(QT_LIB, QT_LIB_LIST) {
        QMAKE_POST_LINK += && $$QMAKE_COPY --dereference $$[QT_INSTALL_LIBS]/$$QT_LIB $$QT_DEST_LIBDIR
    }

    # libusb

    LIBUSB_LIB = /usr/lib/x86_64-linux-gnu/libusb-1.0.so
    exists($$LIBUSB_LIB) {
        QMAKE_POST_LINK += && $$QMAKE_COPY $$LIBUSB_LIB $$DEST_LIBDIR
    } else {
        error($$LIBUSB_LIB doesnt exist)
    }
}
