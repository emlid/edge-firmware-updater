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
public:
    RpiBootImpl(int vid, QList<int> const& pid);
    virtual ~RpiBootImpl(void);
private:
    virtual int bootAsMassStorage_core(void) override;
    std::unique_ptr<RpiBootPrivate> _pimpl;
};

#endif // RPIBOOTIMPL_H