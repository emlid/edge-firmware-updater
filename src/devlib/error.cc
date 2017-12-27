#include "error.h"

#include <errno.h>
#include <QtCore>

int devlib::error(void) {
    return errno;
}


QString devlib::errorDescription(void) {
    return strerror(errno);
}
