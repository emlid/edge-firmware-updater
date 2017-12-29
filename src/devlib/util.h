#ifndef UTIL_H
#define UTIL_H

#include <QtCore>
#if defined(Q_OS_WIN)
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace dbg {
#if defined(Q_OS_WIN)
    inline void debugWindowsError(void) {
        int cache = ::GetLastError();
        qWarning() << "\n!!! (win) GetLastError code: " << cache << "\n";
    }
#else
    inline void debugLinuxError(void) {
        int cache = errno;
        qWarning() << "\n!!! (linux) errno code: " << cache << "\n"
               << "!!! (linux) error description: " << strerror(cache);
    }
#endif

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
