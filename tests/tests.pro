TEMPLATE = subdirs

CONFIG += c++14

SUBDIRS += \
    util_tests \
    rpiboot_cli \
    edge_tests \
    edge_cli \
    devlib_cli \
    flash_cli

!EXCLUDE_CLIENT {
    SUBDIRS += client_tests
}

