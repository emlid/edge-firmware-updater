#include "testnotifier.h"

TestNotifier::TestNotifier(Flasher const& flasher, QObject *parent) : QObject(parent)
{
    connect(&flasher, &Flasher::flashStarted, this, &TestNotifier::onFlashStarted);
    connect(&flasher, &Flasher::flashCompleted, this, &TestNotifier::onFlashCompleted);
    connect(&flasher, &Flasher::progressChanged, this, &TestNotifier::onProgressChanged);
    connect(&flasher, &Flasher::flashAborted, this, &TestNotifier::onFlashAborted);
}
