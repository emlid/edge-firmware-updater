#ifndef RPIBOOTIMPL_H
#define RPIBOOTIMPL_H

#include <memory>
#include <QList>

#include "../RpiBoot.h"

namespace usb {
    class RpiBootImpl;
}

class RpiBootPrivate;

class usb::RpiBootImpl : public usb::IRpiBoot
{
    Q_OBJECT
public:
    RpiBootImpl(int vid, QList<int> const& pid, QObject* parent = nullptr);
    virtual ~RpiBootImpl(void) override;

private:

    virtual int bootAsMassStorage_core(void) override;
    std::unique_ptr<RpiBootPrivate> _pimpl;
};

#endif // RPIBOOTIMPL_H
