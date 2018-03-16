#include "EdgeDeviceImpl.h"
#include "devlib.h"
#include "util.h"

namespace impl {
    auto findPartition(
            devlib::IStorageDeviceInfo* storageDeviceInfo,
            QString const& partLabel
    ) -> std::unique_ptr<devlib::IPartition>;

    auto temporaryMountTo(
            devlib::IPartition* partition,
            QString const& dirName,
            edge::EdgeDeviceImpl::DirFactoryFunction_t const& dirFactory
    ) -> std::unique_ptr<devlib::IMountpoint>;

    auto parseVersionFile(
            QString const& versionFileName,
            edge::EdgeDeviceImpl::FileFactoryFunction_t const& fileFactory
    ) -> QString;
}


auto impl::findPartition(
        devlib::IStorageDeviceInfo* storageDeviceInfo,
        QString const& partLabel) -> std::unique_ptr<devlib::IPartition>
{
    Q_ASSERT(storageDeviceInfo); Q_ASSERT(!partLabel.isEmpty());

    auto partitionsFilter = [&partLabel] (auto const& part) {
        return part->label() == partLabel;
    };

    auto edgeParts = storageDeviceInfo->partitions();
    auto neededPart = std::find_if(edgeParts.begin(),
                                   edgeParts.end(),
                                   partitionsFilter);

    if (neededPart == edgeParts.end()) {
        return {};
    } else {
        auto result = std::move(*neededPart);
        edgeParts.erase(neededPart);
        return result;
    }
}


auto impl::temporaryMountTo(
        devlib::IPartition* partition,
        QString const& dirName,
        edge::EdgeDeviceImpl::DirFactoryFunction_t const& dirFactory
) -> std::unique_ptr<devlib::IMountpoint>
{
    Q_ASSERT(partition); Q_ASSERT(!dirName.isEmpty());

    auto dirPath = QStandardPaths::
            writableLocation(QStandardPaths::TempLocation) + '/' + dirName;

    if (!dirFactory(dirPath)) {
        qWarning() << "can not make directory: " << dirPath;
        return {};
    }

    return partition->mount(dirPath);
}


auto impl::parseVersionFile(const QString &versionFileName,
                            edge::EdgeDeviceImpl::FileFactoryFunction_t const& fileFactory)
    -> QString
{
    Q_ASSERT(!versionFileName.isEmpty());
    auto versionFile = fileFactory(versionFileName);
    auto success = versionFile->open(QIODevice::ReadOnly);

    if (!success) {
        qWarning() << "can not open version file: "
                   << versionFileName;
        return {};
    }

    auto const versionMask = "^v\\d\\.\\d";
    auto regexp = QRegExp(versionMask);
    auto fwVersion = QString();

    while (!versionFile->atEnd()) {
        auto line = versionFile->readLine();
        auto pos = 0;

        if (regexp.indexIn(line, pos) != -1) {
             fwVersion = regexp.capturedTexts().at(0);
             break;
        }
    }

    return fwVersion;
}


auto edge::EdgeDeviceImpl::isEdgeStillAvailable_core(void) const
    -> bool
{
    return true;
}


auto edge::EdgeDeviceImpl::firmwareVersion_core(void) const
    -> QString
{
    auto partWithVersionFile =
            impl::findPartition(_storageDeviceInfo.get(),
                                _config.partitionWithVersiontFile());
    if (!partWithVersionFile) {
        return undefinedVersion();
    }

    using MntptPtr = std::unique_ptr<devlib::IMountpoint>;
    auto mntpt = util::poll<MntptPtr>(
        [&partWithVersionFile] () -> MntptPtr {
            auto mntpts = partWithVersionFile->mountpoints();
            if (mntpts.empty()) {
                return {};
            } else {
                auto result = std::move(mntpts.front());
                mntpts.erase(mntpts.begin());
                return result;
            }
        }
    );

    auto parse = [this] (auto mntptPath){
        auto versionFilePath = mntptPath + '/' + _config.versionFileName();
        auto version = impl::parseVersionFile(versionFilePath, _fileFactory);

        return version.isEmpty() ?
            undefinedVersion() : version;
    };

    if (mntpt) {
        auto mntptPath = mntpt->fsPath();
        return parse(mntptPath);
    } else {
        auto tempMntpt = impl::temporaryMountTo(
                        partWithVersionFile.get(),
                        _config.mntptPathForBootPart(),
                        _dirFactory);

        if (!tempMntpt->isMounted()) {
            qWarning() << "can not mount partition: "
                       << partWithVersionFile->filePath();
            return undefinedVersion();
        }

        auto version = parse(tempMntpt->fsPath());
        tempMntpt->umount();

        return version;
    }

}


auto edge::EdgeDeviceImpl::asIODevice_core(void) const
    -> std::unique_ptr<QIODevice>
{

    auto sdService = devlib::StorageDeviceService::instance();
    return std::unique_ptr<QIODevice>(
        sdService->makeStorageDeviceFile(_storageDeviceInfo->filePath(),
                                         _storageDeviceInfo)
    );
}
