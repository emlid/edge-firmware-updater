#ifndef UTIL_H
#define UTIL_H

#include <QtCore>
#include <unistd.h>

namespace dbg {
    inline void debugLinuxError(void) {
        int cache = errno;
        qWarning() << "\n!!! (linux) errno code: " << cache << "\n"
               << "!!! (linux) error description: " << strerror(cache);
    }

    template<typename T> QString qEnumValueToString(T const enumValue) {
        return QMetaEnum::fromType<T>().valueToKey(enumValue);
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
