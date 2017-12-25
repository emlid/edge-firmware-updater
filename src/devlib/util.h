#ifndef UTIL_H
#define UTIL_H

#include <QtCore>
#include <unistd.h>

namespace dbg {
    inline QDebug qLinuxCritical(void) {
        return qCritical() << " (errno: " << ::strerror(errno) << ") ";
    }
}

namespace fmt {
    inline QString beg(void) {
        return "{";
    }

    inline QString end(void) {
        return "}";
    }

    inline QString sep(void) {
        return ",";
    }

    inline QString tab(void) {
        return "  ";
    }

    inline QString rshift(QString info) {
        QString buf;
        QTextStream in(&info);
        QTextStream out(&buf);

        while(!in.atEnd()) {
            out << in.readLine().prepend(fmt::tab());
        }

        return buf;
    }
}

#endif // UTIL_H
