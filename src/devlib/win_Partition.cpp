#include "Partition.h"
#include "Mountpoint.h"
#include "util.h"
#include "error.h"

using namespace devlib;


class impl::Partition_Private
{
public:
    Partition_Private(QString const& filePath, QString const& fstype, QString const& label)
        : _filePath(filePath), _fstype(fstype), _label(label)
    { }

    QString _filePath;
    QString _fstype;
    QString _label;
};


Partition::Partition(QString const& filePath, QString const& fstype, QString const& label)
    : _pimpl(new impl::Partition_Private(filePath, fstype, label))
{ }


Partition::Partition(Partition&& part) noexcept
    : _pimpl(new impl::Partition_Private(std::move(*part._pimpl)))
{ }


Partition::Partition(Partition const& part)
    : _pimpl(new impl::Partition_Private(*part._pimpl))
{ }


Partition& Partition::operator =(Partition&& part) noexcept
{
    _pimpl.reset(new impl::Partition_Private(std::move(*part._pimpl)));
    return *this;
}


Partition& Partition::operator =(Partition const& part)
{
    _pimpl.reset(new impl::Partition_Private(*part._pimpl));
    return *this;
}


Partition::~Partition(void)
{ }


QString Partition::info(void) const noexcept
{
    return "";
}


QString Partition::filePath(void) const noexcept
{
    Q_ASSERT(_pimpl);
    return _pimpl->_filePath;
}


QString Partition::label(void) const noexcept
{
    Q_ASSERT(_pimpl);
    return _pimpl->_label;
}


// stub
Mountpoint Partition::mount(QString const& path)
{
    Q_UNUSED(path);
    return Mountpoint("");
}


QList<Mountpoint> Partition::mountpoints(void) const
{
    return {};
}
