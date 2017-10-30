HEADERS += $$PWD/WindowsStorageDevice.h \
           $$PWD/WindowsStorageDeviceManager.h

SOURCES += $$PWD/WindowsStorageDevice.cpp \
           $$PWD/WindowsStorageDeviceManager.cpp

#QMAKE_LFLAGS += '\"/MANIFESTUAC:level=\'highestAvailable\' uiAccess=\'false\'\"'

LIBS += -lsetupAPI
