# Firmware Upgrader
> Flash images to USB drives.

***
## Overview
This application was designed for flashing usb devices. General purpose - raspberry device (**Emlid Edge**). 

From client application:
+ Execute firmware upgrader process
+ Call function **start** with firmware image file name as argument

After these steps firmware upgrader:
+ Check for rpi devices in the system
+ Run rpiboot if device is available, and device scanner for searching our device as mass storage
+ Flash device with previously selected firmware image file
+ Check on correctness (Compute checksum of data in the device and compare with image checksum)

_Interprocess API for this application you can see in src/FirmwareUpgraderWatcher.rep_

## Supported Operating Systems

- Linux (most distros)
- Microsoft Windows 7 and later

## Build Steps
### Prerequirements
+ **Qt 5.9.1** or later with **QtRemoteObjects** framework
+ Linux:
  - Ubuntu, Debian: ``` sudo apt-get install libusb-1.0-0-dev libudev-dev ```
+ Windows:
  - ![libusb-1.0.0](https://sourceforge.net/projects/libusb-win32/)

### Build
  ``` qmake firmwareupgrader.pro ```
  
## Usage in other QT projects
+ For interaction with Firmware Upgrader you should:
  - build Firmware Upgrader project as executable
  - include to your project file QtRemoteObjects framework
  
    ```QT += remoteobjects```
  - add to your project FirmwareUpgraderWatcher.rep .
    (this is replica file which will be compiled in c++ class automatically by qt repc compiler)
    this class used for interact with Firmware Upgrader.
    
    ```REPC_REPLICA = FirmwareUpgraderWatcher.rep```
  - run from your application Firmware Upgrader binary with administrative priviledges
  
    ```QProcess::start("gksudo " + QString("path/to/fw/upgrader/executable")); ```
