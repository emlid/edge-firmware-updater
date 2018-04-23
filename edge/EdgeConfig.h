#ifndef EDGECONFIG_H
#define EDGECONFIG_H

#include <QString>

namespace edge {
    class EdgeConfig;
}

class edge::EdgeConfig {
public:
    EdgeConfig(int vid, int pid,
               int pidAsMassStorage,
               int heartbeatPeriod,
               QString const& versionFileName,
               QString const& mntptPathForBootPart,
               QString const& partWithVersionFile)
        : _vid(vid), _pid(pid),
          _pidAsMassStorage(pidAsMassStorage),
          _heartbeatPeriod(heartbeatPeriod),
          _versionFileName(versionFileName),
          _mntptPathForBootPart(mntptPathForBootPart),
          _partWithVersionFile(partWithVersionFile)
    { }

    auto vendorId(void)  const { return _vid; }
    auto productId(void) const { return _pid; }
    auto heartbeatPeriod(void) const { return _heartbeatPeriod; }

    auto productIdAsMassStorage(void)           const { return _pidAsMassStorage; }
    auto const& versionFileName(void)           const { return _versionFileName; }
    auto const& partitionWithVersiontFile(void) const { return _partWithVersionFile; }
    auto const& mntptPathForBootPart(void)      const { return _mntptPathForBootPart; }

private:
    int _vid, _pid, _pidAsMassStorage;
    int _heartbeatPeriod;
    QString _versionFileName,
            _mntptPathForBootPart,
            _partWithVersionFile;
};

#endif // EDGECONFIG_H
