#include "CRCServiceImpl.h"
#include "../util.h"

#include <cstdlib>
#include <QCryptographicHash>
#include <QtCore>


std::pair<QByteArray, util::CRCServiceImpl::RetStatus>
    util::CRCServiceImpl::computeCRC_core(QIODevice *source,
                                          qint64 length,
                                          CancellationCondition_t cancellationCondition,
                                          ProgressReporter_t progressReporter,
                                          qint64 ioBlockSize)
{
    if (length <= 0) {
        qWarning() << "length is less or equal to 0";
        return {{}, RetStatus::ReadFail};
    } else if (!source->isOpen() || !source->isReadable()) {
        qWarning() << "source did not open as readable";
        return {{}, RetStatus::ReadFail};
    } else if (ioBlockSize <= 0) {
        qWarning() << "ioBlockSize less or equal to 0";
        return {{}, RetStatus::ReadFail};
    }

    QCryptographicHash hash(QCryptographicHash::Md5);

    std::lldiv_t res = std::div(length, ioBlockSize);
    auto const& blocksForRead = res.quot;
    auto const& remBytes      = res.rem;

    auto prevProgress = 0LL;

    for (auto i = 0LL; i < blocksForRead; i++) {
        if (cancellationCondition()) {
            return {{}, RetStatus::Cancelled};
        }

        auto data = source->read(ioBlockSize);
        if (data.size() != ioBlockSize) {
            return {{}, RetStatus::ReadFail};
        }

        hash.addData(data);

        auto currentProgress = util::computePercent(i * ioBlockSize, length);
        if (prevProgress != currentProgress) {
            progressReporter(i * ioBlockSize, length);
            prevProgress = currentProgress;
        }
    }

    if (cancellationCondition()) {
        return {{}, RetStatus::Cancelled};
    }

    if (remBytes > 0) {
        auto data = source->read(remBytes);
        if (data.size() != remBytes) {
            return {{}, RetStatus::ReadFail};
        }

        hash.addData(data);
        progressReporter(length, length);
    }

    return {hash.result(), RetStatus::Success};
}
