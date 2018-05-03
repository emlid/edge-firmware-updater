#include "utilities.h"

#if defined(Q_OS_LINUX) || defined(Q_OS_MACX)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace logg {
    Q_LOGGING_CATEGORY(basic, "main");
}

void updater::perm::reduceRObjectNodePermissions(QString const& remoteObjNodeName)
{
#ifndef Q_OS_MACX
    Q_UNUSED(remoteObjNodeName);
#else
    constexpr auto sudoUidVar = "SUDO_UID";
    constexpr auto sudoGidVar = "SUDO_GID";

    struct {
        uid_t uid  = UINT32_MAX;
        uid_t euid = UINT32_MAX;
        gid_t gid  = UINT32_MAX;
    } procInfo;

    auto env = QProcessEnvironment::systemEnvironment();

    if (env.contains(sudoUidVar) && env.contains(sudoGidVar)) {
        procInfo.uid  = env.value(sudoUidVar).toUInt();
        procInfo.gid  = env.value(sudoGidVar).toUInt();
        procInfo.euid = ::geteuid();
    } else {
        qCWarning(logg::basic()) << "Firmware updater process should "
                                    "be executed with administrator privileges";
    }

    auto socketPath = QString(remoteObjNodeName).replace("local:", "");
    ::chown(socketPath.toStdString().data(), procInfo.uid, procInfo.gid);

    qCInfo(logg::basic())
            << "Process info\n"
            << "- uid: "  << (procInfo.uid  == UINT32_MAX ? ::getuid()  : procInfo.uid)  << "\n"
            << "- euid: " << (procInfo.euid == UINT32_MAX ? ::geteuid() : procInfo.euid) << "\n"
            << "- gid: "  << (procInfo.gid  == UINT32_MAX ? ::getgid()  : procInfo.gid)  << "\n"
            << "- pid: "  << ::getpid() << "\n";
#endif
}


void updater::perm::reducePriviledge(void)
{
#if defined(Q_OS_LINUX) || defined(Q_OS_MACX)
    ::umask(0);
#endif
}


auto updater::logFilePath(void) -> QString
{
    auto const logName = QString{"fwugprader.log"};
#ifdef Q_OS_WIN
    return QString("C:\\Temp\\%1").arg(logName);
#else
    return QString("/tmp/%1").arg(logName);
#endif
}


void updater::messageHandler(QFile* logFile,
                             QtMsgType msgType,
                             QMessageLogContext const& ctx,
                             QString const& msg)
{
    auto log = [logFile] (auto msg) {
        if (logFile) { (QTextStream(logFile) << msg).flush(); }
        (QTextStream(stderr) << msg).flush();
    };

    auto sendMsgWithType =
        [&msg, log, &ctx]
            (auto const& type) -> void {
                auto curTime = QTime::currentTime().toString();
                auto logMessage = QString("%1: %2: %3: %4\n")
                        .arg(curTime).arg(ctx.category).arg(type).arg(msg);

                log(logMessage);
            };

    switch (msgType) {
        case QtDebugMsg:    sendMsgWithType("debug");    break;
        case QtCriticalMsg: sendMsgWithType("critical"); break;
        case QtFatalMsg:    sendMsgWithType("fatal");    std::abort();
        case QtWarningMsg:  sendMsgWithType("warn");     break;
        case QtInfoMsg:     sendMsgWithType("info");     break;
    }
}
