#ifndef OPTIONAL_H
#define OPTIONAL_H

#include <QObject>

template<typename T>
class Optional : public QObject
{
    Q_OBJECT
public:
    Optional(QObject *parent = nullptr);

    static Optional<T> of(T const& value, bool isPresent = true, bool isCritical = false);

    bool isPresent(void);

    bool isErrorCritical();

    QString getErrorMessage();

    T getValue();

private:
    T _value;
    bool _isPresent;
    bool _isCritical;
};

#endif // OPTIONAL_H
