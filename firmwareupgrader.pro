lessThan(QT_MAJOR_VERSION, 5) {
        error("Required at least Qt 5.9.")
}

equals(QT_MAJOR_VERSION, 5) : lessThan(QT_MINOR_VERSION, 9) {
        error("Required at least Qt 5.9.")
}

message(Qt version: $$[QT_VERSION])


QT += core testlib remoteobjects
QT -= gui

CONFIG += c++14

TARGET = fwupgrader
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

OBJECTS_DIR = obj
MOC_DIR     = moc
RCC_DIR     = resources

debug {
    DESTDIR = $${OUT_PWD}/debug
} else {
    DESTDIR = $${OUT_PWD}/release
}

include(common.pri)

contains(CONFIG, DISABLE_BUILD_WITH_LIBS) {
    message("Skip building with libs")
} else {
    include(libs.pri)
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
