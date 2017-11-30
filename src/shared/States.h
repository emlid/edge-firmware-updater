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
    RpiBootCancelled,
    RpiBootFinished,
    RpiBootFailed
};


enum DeviceScannerState {
    ScannerDeviceFound = 0,
    ScannerDeviceNotFound,
    ScannerStarted,
    ScannerCancelled,
    ScannerFinished,
    ScannerFailed
};


enum FlasherState {
    FlasherImageReadingFailed = 0,
    FlasherDeviceWritingFailed,
    FlasherOpenImageFailed,
    FlasherOpenDeviceFailed,
    FlasherStarted,
    FlasherCancelled,
    FlasherFinished,
    FlasherFailed
};


enum CheckingCorrectnessState {
    CheckingCorrectnessStarted,
    ComputeImageChecksum,
    ComputeDeviceChecksum,
    ReadingFailed,
    ImageCorrectlyWrote,
    CheckingCancelled,
    ImageUncorrectlyWrote
};

}

#endif // STATES_H
