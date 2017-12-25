#include <blkid/blkid.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <unistd.h>

#include "Partition.h"
#include "Mountpoint.h"
#include "util.h"

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


Partition::Partition(Partition&& part)
    : _pimpl(new impl::Partition_Private(std::move(*part._pimpl)))
{ }


Partition::Partition(Partition const& part)
    : _pimpl(new impl::Partition_Private(*part._pimpl))
{ }


Partition Partition::operator =(Partition&& part)
{
    return Partition(std::move(part));
}


Partition Partition::operator =(Partition const& part)
{
    return Partition(part);
}


Partition::~Partition(void)
{ }


QString Partition::info(void) const noexcept
{
    auto info = QString();

    QTextStream(&info)
            << fmt::beg()
            << " name: "   << _pimpl->_filePath << fmt::sep()
            << " fstype: " << _pimpl->_fstype   << fmt::sep()
            << " label: "  << _pimpl->_label    << " " << fmt::end();

    return info;
}


QString Partition::filePath(void) const noexcept
{
    return _pimpl->_filePath;
}


QString Partition::label(void) const noexcept
{
    return _pimpl->_label;
}


Mountpoint Partition::mount(QString const& path)
{
    auto partition = _pimpl->_filePath.toStdString();
    auto dest      = path.toStdString();
    auto fstype    = _pimpl->_fstype.toStdString();

    auto result = ::mount(partition.data(), dest.data(), fstype.data(), 0, nullptr);

    if (result != 0) {
        dbg::qLinuxCritical() << "can not mount " << partition.data() << " to " << dest.data();
        return Mountpoint("");
    }

    return Mountpoint(path);
}


QList<Mountpoint> Partition::mountpoints(void) const
{
    auto mntpts = QStorageInfo::mountedVolumes();
    auto partMntpts = QList<Mountpoint>();

    for (auto& mntpt : mntpts) {
        if (mntpt.device() == _pimpl->_filePath) {
            partMntpts.push_back(Mountpoint(mntpt.rootPath()));
        }
    }

    return partMntpts;
}
