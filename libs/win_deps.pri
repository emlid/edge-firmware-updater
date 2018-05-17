win32 {
    # First. Copy libusb
    CONFIG(release, debug|release): BUILD_TYPE = release
    else:CONFIG(debug, debug|release): BUILD_TYPE = debug

    WIN_LIBRARY_ROOT_PATH = "C:"
    LIBUSB_PATH =  $$shell_path("$${WIN_LIBRARY_ROOT_PATH}/libusb")

    QMAKE_POST_LINK += $$escape_expand(\\n) $$QMAKE_COPY \
        \"$$shell_path($${LIBUSB_PATH}/MS32/dll/libusb-1.0.dll)\" \
        \"$$shell_path($$OUT_PWD/$$BUILD_TYPE/)\"

    # Second. Copy all needed Qt libraries with 'windeployqt' command
    QT_BIN_DIR       = $$dirname(QMAKE_QMAKE)
    DEPLOY_TARGET    = $$shell_quote($$shell_path($$OUT_PWD/$${BUILD_TYPE}/$${TARGET}.exe))
    QMAKE_POST_LINK += && "$$QT_BIN_DIR/windeployqt" --no-compiler-runtime $${DEPLOY_TARGET}
}
