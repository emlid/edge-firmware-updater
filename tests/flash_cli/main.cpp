#include <QCoreApplication>
#include <QtCore>

#include <edge.h>
#include <util.h>

#include "../../main/shared/shared.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    struct {
        bool skipFlashing;
        bool skipChecksum;
    } flags = {false, false};

    auto image_idx = 1;

    if (argc >= 1) {
        if (argc >= 2) {
            if (QString(argv[1]) == "--skip-flash") {
                flags.skipFlashing = true;
                image_idx++;
            }
            if (argc >= 3) {
                if (QString(argv[2]) == "--skip-crc") {
                    flags.skipChecksum = true;
                    image_idx++;
                }
            }
        }
    } else {
        qWarning() << "too few args. Required 1.";
        return EXIT_FAILURE;
    }

    namespace conf = edge::tag;
    auto config = edge::EdgeConfig(
                conf::Vid{0xa5c},
                conf::Pid{0x2764},
                conf::Pid{0x0001},
                conf::VersionFileName{"issue.txt"},
                conf::MntptPathForBootPart{"temp_mnt"},
                conf::PartWithVersionFile{"boot"}
    );

    auto edgeManager = edge::makeEdgeManager(config);

    while(!(edgeManager->isEdgePlugged() || edgeManager->isEdgeInitialized())) {
        QThread::msleep(300);
        QTextStream(stdout) << "\rwaiting for edge...";
    }

    qInfo() << "Edge found. Initializing...";
    auto edgeDevice = edgeManager->initialize();

    if (!edgeDevice) {
        qCritical() << "Initialization failed";
        return EXIT_FAILURE;
    }

    // print info

    qInfo() << "fw version: " << edgeDevice->firmwareVersion();
    auto iodevice = edgeDevice->asIODevice();

    if (!iodevice) {
        qCritical() << "Can not create IO device";
        return EXIT_FAILURE;
    }

    QFile image(argv[image_idx]);
    if (!image.open(QIODevice::ReadOnly)) {
        qWarning() << "can not open image file";
        return EXIT_FAILURE;
    }
    qInfo() << "image opened";

    if (!iodevice->open(QIODevice::ReadWrite)) {
        qWarning() << "can not open device file";
        return EXIT_FAILURE;
    }


    auto progressReporter =
        [] (auto const& curr, auto const& total) {
            QTextStream(stdout) << QString("\r%1 %").arg((curr * 100) / total);
        };

    if (!flags.skipFlashing) {
        qInfo() << "Flashing started";

        auto flashingService = util::makeFlashingService();
        auto result = flashingService->flash(
            &image, iodevice.get(),
            util::IOService::defaultCancellationCondition(),
            progressReporter
        );

        if (result != util::IOService::RetStatus::Success) {
            qWarning() << "Flashing failed";
            if (result == util::IOService::RetStatus::Cancelled) {
                qWarning() << "Cancelled";
            }

            return EXIT_FAILURE;
        }
    }

    if (!flags.skipChecksum) {
        auto crcService = util::makeCRCService();
        qInfo() << "compute image crc";

        image.seek(0);
        auto imageCrc = crcService->computeCRC(
            &image, image.size(),
            util::IOService::defaultCancellationCondition(),
            progressReporter
        );

        qInfo() << "compute device crc";

        iodevice->close();
        iodevice->open(QIODevice::ReadOnly);

        auto deviceCrc = crcService->computeCRC(
            iodevice.get(), image.size(),
            util::IOService::defaultCancellationCondition(),
            progressReporter
        );

        if (deviceCrc.second != util::IOService::RetStatus::Success) {
            qInfo() << "not success";
        }

        if (imageCrc == deviceCrc) {
            qInfo() << "image successfully wrote";
            return EXIT_SUCCESS;
        } else {
            qWarning() << "image incorrectly wrote";
            return EXIT_FAILURE;
        }
    }
}
