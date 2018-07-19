lessThan(QT_MAJOR_VERSION, 5) {
        error("Required at least Qt 5.9.")
}

equals(QT_MAJOR_VERSION, 5) : lessThan(QT_MINOR_VERSION, 9) {
        error("Required at least Qt 5.9.")
}

message(Qt version: $$[QT_VERSION])

CONFIG -= debug_and_release

CONFIG(debug,   debug|release):message(Debug build)
else:message(Release build)

win32:message(Windows build)
linux:message(Linux build)
macx :message(OSX build)

!linux:!win32:!macx:error(Unsupported OS)

TEMPLATE = subdirs

!NO_LIBS:include(libs/libs.pri)

SUBDIRS += \
    devlib \
    rpi \
    util \
    main \
    edge \

devlib.subdir = devlib/devlib
edge.depends = rpi util devlib
main.depends = edge

!EXCLUDE_TESTS {
    SUBDIRS += tests
    tests.depends = edge
}

!EXCLUDE_CLIENT {
    SUBDIRS += client
    tests.depends += client
}

