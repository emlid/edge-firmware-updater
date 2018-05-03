#ifndef EDGECONFIG_H
#define EDGECONFIG_H

#include <QString>

namespace edge {
    class EdgeConfig;

    template<typename T>
    struct ConfigTag {
        constexpr explicit ConfigTag(T const& value)
            : _value(value)
        { }

        T const& get() const { return _value; }
        T& get() { return _value; }

    private:
        T _value;
    };

    namespace tag {
        struct Vid :      public ConfigTag<int> { constexpr Vid(int value) : ConfigTag(value) {} };
        struct Pid :      public ConfigTag<int> { constexpr Pid(int value) : ConfigTag(value) {} };

        struct VersionFileName :      public ConfigTag<QString>
            { constexpr VersionFileName(QString const& value) : ConfigTag(value) {} };

        struct MntptPathForBootPart : public ConfigTag<QString>
            { constexpr MntptPathForBootPart(QString const& value) : ConfigTag(value) {} };

        struct PartWithVersionFile :  public ConfigTag<QString>
            { constexpr PartWithVersionFile(QString const& value) : ConfigTag(value) {} };
    };
}

class edge::EdgeConfig {
public:
    EdgeConfig(tag::Vid vid,
               tag::Pid pid,
               tag::Pid pidAsMassStorage,
               tag::VersionFileName      const& versionFileName,
               tag::MntptPathForBootPart const& mntptPathForBootPart,
               tag::PartWithVersionFile  const& partWithVersionFile)
        : _vid(vid), _pid(pid),
          _pidAsMassStorage(pidAsMassStorage),
          _versionFileName(versionFileName),
          _mntptPathForBootPart(mntptPathForBootPart),
          _partWithVersionFile(partWithVersionFile)
    { }

    auto vendorId(void)  const { return _vid.get(); }
    auto productId(void) const { return _pid.get(); }

    auto productIdAsMassStorage(void)           const { return _pidAsMassStorage.get(); }
    auto const& versionFileName(void)           const { return _versionFileName.get(); }
    auto const& partitionWithVersiontFile(void) const { return _partWithVersionFile.get(); }
    auto const& mntptPathForBootPart(void)      const { return _mntptPathForBootPart.get(); }

private:
    tag::Vid _vid;
    tag::Pid _pid;
    tag::Pid _pidAsMassStorage;

    tag::VersionFileName      _versionFileName;
    tag::MntptPathForBootPart _mntptPathForBootPart;
    tag::PartWithVersionFile  _partWithVersionFile;
};

#endif // EDGECONFIG_H
