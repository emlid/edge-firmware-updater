#include <unistd.h>

#include "util.h"

QDebug qCriticalWithErrno(void)
{
    return qCritical() << "(Code: " << errno << ") ";
}
