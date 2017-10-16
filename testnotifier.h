#ifndef TESTNOTIFIER_H
#define TESTNOTIFIER_H

#include <QObject>

#include "Flasher.h"

class TestNotifier : public QObject
{
    Q_OBJECT
public:
    TestNotifier(QIOFlasher const& flasher, QObject *parent = nullptr);

public slots:
    void onFlashStarted() {
        QTextStream(stdout) << "Flashing started." << '\n';
    }

    void onFlashCompleted() {
        QTextStream(stdout) << "Flash completed." << '\n';
    }

    void onProgressChanged(uint progress) {
        QTextStream(stdout) << "\rProgress: " << progress << "%";
    }

    void onFlashAborted(QIOFlasher::FlashingStatus status) {
        QString res = status == QIOFlasher::FlashingStatus::READ_FAILED ? "READ" : "WRITE";
        QTextStream(stdout) << "Flash aborted" << res << '\n';
    }
};

#endif // TESTNOTIFIER_H
