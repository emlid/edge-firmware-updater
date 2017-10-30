#ifndef EXITSTATUS_H
#define EXITSTATUS_H

#include <QtCore>

class ExecutionStatus {
public:
    static const ExecutionStatus SUCCESS;

    ExecutionStatus(int errorCode, bool isCritical = false);

    ExecutionStatus(int errorCode, QString const& additionalMessage, bool isCritical = false);

    bool failed();

    bool isCritical(void);

    QString errorMessage(void);

private:
    bool _failed;
    bool _isCritical;
    QString _errorMessage;

};

#endif // EXITSTATUS_H
