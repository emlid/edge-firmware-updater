#ifndef EXITSTATUS_H
#define EXITSTATUS_H

#include <QtCore>

class ExitStatus {
public:
    static const ExitStatus SUCCESS;

    ExitStatus(int errorCode, bool isCritical = false);

    ExitStatus(int errorCode, QString const& additionalMessage, bool isCritical = false);

    bool failed();

    bool isCritical(void);

    QString errorMessage(void);

private:
    bool _failed;
    bool _isCritical;
    QString _errorMessage;

};

#endif // EXITSTATUS_H
