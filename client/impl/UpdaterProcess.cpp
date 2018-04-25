#include "UpdaterProcess.h"


client::UpdaterProcess::UpdaterProcess(void)
{ }


static void linuxStartRootProcess(QProcess* proc,
                                  QString const& programPath,
                                  QStringList const& args,
                                  QProcess::OpenMode const& mode)
{
    constexpr auto graphicalSudo = "pkexec";

    auto argsStr = args.join(' ');
    auto argsEnvVar = QString("FW_ARGS=%1").arg(argsStr);
    auto progEnvVar = QString("FW_PROGRAM=%1").arg(programPath);
    auto sudoEnvVar = QString("FW_GSUDO=%1").arg(graphicalSudo);

    auto env = proc->environment();
    env.append(argsEnvVar);
    env.append(progEnvVar);
    env.append(sudoEnvVar);
    proc->setEnvironment(env);

    proc->setProgram("/bin/sh");
    proc->setArguments({"-c", "${FW_GSUDO} \"${FW_PROGRAM}\" ${FW_ARGS}"});
    proc->open(mode);
}


static void macxStartRootProcess(QProcess* proc,
                                 QString const& program,
                                 QStringList const& args,
                                 QProcess::OpenMode const& mode)
{
    auto argsStr = args.join(' ');
    auto argsEnvVar = QString("FW_ARGS=%1").arg(argsStr);
    auto progEnvVar = QString("FW_PROGRAM=%1").arg(program);

    auto env = proc->environment();
    env.append(argsEnvVar);
    env.append(progEnvVar);
    proc->setEnvironment(env);

    auto const script =
        QString("do shell script \"\\\"${FW_PROGRAM}\\\" ${FW_ARGS}\" "
                "with administrator privileges");

    proc->setProgram("osascript");
    proc->setArguments({"-e", script});
    proc->open(mode);
}


static void winStartRootProcess(QProcess* proc,
                                QString const& program,
                                QStringList const& args,
                                QProcess::OpenMode const& mode)
{
    auto argsStr = args.join(' ');
    auto argsEnvVar = QString("FW_ARGS=%1").arg(argsStr);
    auto progEnvVar = QString("FW_PROGRAM=\"%1\"").arg(program);

    auto env = proc->environment();
    env.append(argsEnvVar);
    env.append(progEnvVar);
    proc->setEnvironment(env);

    proc->start("cmd.exe /C %FW_PROGRAM% %FW_ARGS%", mode);
}


void client::UpdaterProcess::startAsAdmin(QString const& program,
                                          QStringList const& args,
                                          QProcess::OpenMode const& mode)
{
    // suppress clang warnings
    Q_UNUSED(winStartRootProcess);
    Q_UNUSED(linuxStartRootProcess);
    Q_UNUSED(macxStartRootProcess);

    Q_ASSERT(!program.isEmpty());
#ifdef Q_OS_MACX
    macxStartRootProcess(this, program, args, mode);
#elif defined Q_OS_LINUX
    linuxStartRootProcess(this, program, args, mode);
#elif defined Q_OS_WIN32
    winStartRootProcess(this, program, args, mode);
#endif
}
