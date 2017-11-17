#ifndef STATES_H
#define STATES_H

namespace states {

enum StateType {
    Info = 0,
    Error,
    Warning
};


enum RpiBootState {
    RpiBootDeviceFound = 0,
    RpiBootDeviceNotFound,
    RpiBootStarted,
    RpiBootFinished,
    RpiBootFailed
};


enum DeviceScannerState {
    ScannerDeviceFound = 0,
    ScannerDeviceNotFound,
    ScannerStarted,
    ScannerFinished,
    ScannerFailed
};


enum FlasherState {
    FlasherImageReadingFailed = 0,
    FlasherDeviceWritingFailed,
    FlasherOpenImageFailed,
    FlasherOpenDeviceFailed,
    FlasherStarted,
    FlasherFinished,
    FlasherFailed,
    FlasherCheckingCorrectnessStarted,
    FlasherImageCorrectlyWrote,
    FlasherImageUncorrectlyWrote
};

}

#endif // STATES_H
