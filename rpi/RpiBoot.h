#ifndef IRPIBOOT_H
#define IRPIBOOT_H

#include <QtCore>

namespace usb {
    class IRpiBoot;
}

class usb::IRpiBoot : public QObject
{
    Q_OBJECT
public:
    int bootAsMassStorage(void) { return bootAsMassStorage_core(); }
    virtual ~IRpiBoot(void) = default;

signals:
    void infoMessageReceived(QString msg);
    void warnMessageReceived(QString msg);
    void errorMessageReceived(QString msg);

private:
    virtual int bootAsMassStorage_core(void) = 0;

protected:
    IRpiBoot(QObject* parent = nullptr)
        : QObject(parent)
    { }
};



#endif // IRPIBOOT_H
