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
        qCWarning(edge::edgelog()) << "Can not make directory: " << dirPath;
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
        qCWarning(edge::edgelog()) << "Can not open version file: "
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
    qCDebug(edgelog()) << "Search partitions with label "
                       << _config.partitionWithVersiontFile();

    auto partWithVersionFile =
            impl::findPartition(_storageDeviceInfo.get(),
                                _config.partitionWithVersiontFile());
    if (!partWithVersionFile) {
        qCWarning(edgelog()) << "Can not get boot partition";
        return undefinedVersion();
    }

    qCInfo(edgelog()) << "Waiting for automounting boot partition";

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
        qCInfo(edgelog()) << "Partition is automounted by OS. "
                             "Parsing file with firmware version..."
                          << mntpt->fsPath();
        auto mntptPath = mntpt->fsPath();
        return parse(mntptPath);

    } else {
        qCInfo(edgelog()) << "Partition is not automounted. "
                             "Mounting boot partition manually...";

        auto tempMntpt = impl::temporaryMountTo(
                        partWithVersionFile.get(),
                        _config.mntptPathForBootPart(),
                        _dirFactory);

        if (!tempMntpt->isMounted()) {
            qCWarning(edgelog()) << "Can not mount partition: "
                                 << partWithVersionFile->filePath();
            return undefinedVersion();
        }

        qCInfo(edgelog()) << "Partition successfully mounted. "
                             "Parsing firmware version from "
                          << tempMntpt->fsPath();

        auto version = parse(tempMntpt->fsPath());
        tempMntpt->umount();

        qCInfo(edgelog()) << "Firmware version: " << version;

        return version;
    }

}


auto edge::EdgeDeviceImpl::asIODevice_core(void) const
    -> std::unique_ptr<devlib::IStorageDeviceFile>
{

    auto sdService = devlib::StorageDeviceService::instance();
    return std::unique_ptr<devlib::IStorageDeviceFile>(
        sdService->makeStorageDeviceFile(_storageDeviceInfo->filePath(),
                                         _storageDeviceInfo)
    );
}
