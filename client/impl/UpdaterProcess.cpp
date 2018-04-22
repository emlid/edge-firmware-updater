#include "UpdaterProcess.h"
#include "unistd.h"


UpdaterProcess::UpdaterProcess()
{ }



static QString compose(QString const& str) {
    static const QChar quote = '\'';
    return QString(str).append(quote).prepend(quote);
}


static void linuxStartRootProcess(QString const& program)
{
    Q_UNUSED(program);

}


static void macxStartRootProcess(QProcess* proc,
                                 QString const& program,
                                 QProcess::OpenMode const& mode)
{
    auto const graphicalSudo = "osascript";
    auto const exec = "-e";
    auto const script = QString("do shell script \"%1\" with administrator privileges")
        .arg(compose(program));

    proc->setProgram(graphicalSudo);
    proc->setArguments({exec, script});

    proc->open(mode);
}


static void winStartRootProcess(QString const& program)
{
    Q_UNUSED(program);
}


void UpdaterProcess::startAsAdmin(QString const& program,
                              QStringList const& args,
                              QProcess::OpenMode const& mode)
{
    Q_ASSERT(!program.isEmpty());
    macxStartRootProcess(this, program, mode);
}
