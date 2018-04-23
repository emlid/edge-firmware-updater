client {
    REPC_REPLICA = $$PWD/EdgeFirmwareUpdaterIPC.rep
} else {
    REPC_SOURCE = $$PWD/EdgeFirmwareUpdaterIPC.rep
}

HEADERS += \
    $$PWD/shared.h

INCLUDEPATH += $${PWD}
