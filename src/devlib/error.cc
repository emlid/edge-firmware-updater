#include "error.h"

#include <QtCore>

#if defined(Q_OS_WIN)
#include <windows.h>
#else
#include <unistd.h>
#endif

int devlib::error(void) {
#if defined(Q_OS_WIN)
    return ::GetLastError();
#else
    return errno;
#endif
}


QString devlib::errorDescription(void) {
#if defined(Q_OS_WIN)
    return "";
#else
    return strerror(errno);
#endif
}
