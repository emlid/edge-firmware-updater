#include "ExitStatus.h"

const ExitStatus ExitStatus::SUCCESS = ExitStatus(0);

ExitStatus::ExitStatus(int errorCode, bool isCritical)
    : _errorMessage(""), _failed(errorCode != 0), _isCritical(isCritical)
{
}


ExitStatus::ExitStatus(int errorCode, QString const& additionalMessage, bool isCritical)
    :_errorMessage(""), _failed(errorCode != 0), _isCritical(isCritical)
{
    qDebug() << "DEBUG_OUT: " << additionalMessage;
}


bool ExitStatus::failed(void)
{
    return _failed;
}


bool ExitStatus::isCritical(void)
{
    return _isCritical;
}


QString ExitStatus::errorMessage(void)
{
    return _errorMessage;
}
