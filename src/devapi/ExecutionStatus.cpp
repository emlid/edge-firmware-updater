#include "ExecutionStatus.h"


const ExecutionStatus ExecutionStatus::SUCCESS = ExecutionStatus(0);


ExecutionStatus::ExecutionStatus(int errorCode, bool isCritical)
    : _failed(errorCode != 0),
      _isCritical(isCritical),
      _errorMessage("")
{ }


ExecutionStatus::ExecutionStatus(int errorCode, QString const& additionalMessage, bool isCritical)
    : _failed(errorCode != 0),
      _isCritical(isCritical),
      _errorMessage("")
{
    (isCritical ? qCritical() : qDebug()) << additionalMessage;
}


bool ExecutionStatus::failed(void)
{
    return _failed;
}


bool ExecutionStatus::isCritical(void)
{
    return _isCritical;
}


QString ExecutionStatus::errorMessage(void)
{
    return _errorMessage;
}
