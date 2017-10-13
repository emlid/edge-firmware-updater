#ifndef CONSOLE_H
#define CONSOLE_H

#include <QObject>
#include "qioflasher.h"

class Console : public QObject
{
    Q_OBJECT
public:
    explicit Console(QObject *parent = nullptr);

    Console(QIOFlasher* flasher, QObject* parent = nullptr);

public slots:
    void onFlashingCompleted();
    void onFlashingStarted();
    void onFlashingAborted(QIOFlasher::FlashingStatus status);
    void onProgressChanged(uint progress);
};

#endif // CONSOLE_H
