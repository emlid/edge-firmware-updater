HEADERS += windows/WindowsStorageDevice.h \
           windows/WindowsStorageDeviceManager.h

SOURCES += windows/WindowsStorageDevice.cpp \
           windows/WindowsStorageDeviceManager.cpp

#QMAKE_LFLAGS += '\"/MANIFESTUAC:level=\'highestAvailable\' uiAccess=\'false\'\"'

LIBS += -lsetupAPI
