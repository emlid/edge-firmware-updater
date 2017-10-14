#include "testnotifier.h"

TestNotifier::TestNotifier(QIOFlasher const& flasher, QObject *parent) : QObject(parent)
{
    connect(&flasher, &QIOFlasher::flashStarted, this, &TestNotifier::onFlashStarted);
    connect(&flasher, &QIOFlasher::flashCompleted, this, &TestNotifier::onFlashCompleted);
    connect(&flasher, &QIOFlasher::progressChanged, this, &TestNotifier::onProgressChanged);
    connect(&flasher, &QIOFlasher::flashAborted, this, &TestNotifier::onFlashAborted);
}
