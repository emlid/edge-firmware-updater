#include <iostream>

#include "console.h"

    Console::Console(QObject* parent) : QObject(parent) {}

    Console::Console(QIOFlasher* flasher, QObject* parent) : QObject(parent) {
        connect(flasher, &QIOFlasher::flashAborted,    this, &Console::onFlashingAborted);
        connect(flasher, &QIOFlasher::flashStarted,    this, &Console::onFlashingStarted);
        connect(flasher, &QIOFlasher::flashCompleted,  this, &Console::onFlashingCompleted);
        connect(flasher, &QIOFlasher::progressChanged, this, &Console::onProgressChanged);
    }

    void Console::onFlashingCompleted() {
        QTextStream(stdout) << "Flasher: flashing completed successfully.\n";
    }

    void Console::onFlashingStarted() {
        QTextStream(stdout) << "Flasher: start flash\n";
    }

    void Console::onFlashingAborted(QIOFlasher::FlashingStatus status) {
        using FlashStat = QIOFlasher::FlashingStatus ;

        switch (status) {
            case FlashStat::READ_FAILED :
                QTextStream(stdout) << "Error: reading.\n";
                break;

            case FlashStat::WRITE_FAILED :
                QTextStream(stdout) << "Error: writing.\n";
                break;

            default:
                break;
        }
    }

    void Console::onProgressChanged(uint progress) {
        QTextStream(stdout) << "Flasher: " << progress << "%\r";
    }
