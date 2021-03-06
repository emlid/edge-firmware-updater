# Edge Firmware Updater

[![CodeFactor](https://www.codefactor.io/repository/github/emlid/edge-firmware-updater/badge)](https://www.codefactor.io/repository/github/emlid/edge-firmware-updater)
[![Releases](https://img.shields.io/github/release/emlid/edge-firmware-updater.svg)](https://github.com/emlid/QGroundControl/releases)
[![Build Status](https://travis-ci.org/emlid/edge-firmware-updater.svg?branch=master)](https://travis-ci.org/emlid/edge-firmware-updater)
[![Build status](https://ci.appveyor.com/api/projects/status/gk7khg2xvgjj1911/branch/master?svg=true)](https://ci.appveyor.com/project/EmlidBuilderBot/edge-firmware-updater/branch/master)

> Flash firmware images

***

## Overview

This application was designed for flashing usb devices. General purpose - raspberry device (**Emlid Edge**).
Firmware updater generally runs as subprocess of any applications. Interprocess communication implemented via **QtRemoteObjects**.
For use Firmware Updater from your project, you should include `main/shared/shared.pri` into your `pro` file. The project is written on `C++14` standard.

### General functions

+ Find Edge device in the system
+ Boot Edge as mass storage (via rpiboot)
+ Get the version of current firmware
+ Flash image
+ Validate writing (CRC)

## Supported Operating Systems

+ Linux (most distros)
+ Microsoft Windows 7 and later
+ MacOSX

## Build

### Compiler

| OS | Requirements |
| ------ | ------ |
| **Linux** | `g++5` or higher, 64 bit |
| **Windows** | [msvc2015](http://www.visualstudio.com/downloads/download-visual-studio-vs#d-express-windows-desktop), 32 bit |
| **OSX** | `clang++`, 64 bit |

### Dependencies

#### Qt

+ Download the [Qt installer](http://www.qt.io/download-open-source)
+ You need to install `Qt 5.9.3` or higher, and following packages:
  + Qt Core for your compiler (msvc2015/g++/clang++)
  + QtRemoteObjects

#### Additional packages

| OS | Packages |
| ------ | ------ |
| **Linux** | `sudo apt-get install libusb-1.0.0-dev libblkid-dev libudev-dev` |
| **Windows** | Download [libusb](https://github.com/libusb/libusb/releases/download/v1.0.21/libusb-1.0.21.7z):  extract it into the `C:\libusb` directory |
| **OSX** | `brew install libusb` |

**Note**: On Windows you should install drivers for rpiboot. Download `.exe` from
[official usbboot repo](https://github.com/raspberrypi/usbboot/tree/master/win32/redist)
and install drivers for `BCM2708` and `BCM2710` using command line (Run `cmd.exe` as administrator):

``` bash
.\wdi-simple.exe -n "Raspberry Pi USB boot" -v 0x0a5c -p 0x2763 -t 0
.\wdi-simple.exe -n "Raspberry Pi USB boot" -v 0x0a5c -p 0x2764 -t 0
```

### Make

+ From directory with project (use shadow build), type in terminal:
  + sh
    ```bash
    cd ..
    mkdir fwupdater-build
    cd fwupdater-build
    qmake "../edge-firmware-updater/fwupdater.pro"
    make
    ```

  + pwsh (PowerShell)
    ```bash
    cd ..
    mkdir fwupdater-build
    cd fwupdater-build
    qmake "..\edge-firmware-updater\fwupdater.pro"
    jom.exe
    ```

+ **Note**: By default ``qmake`` locates in ``[QTPATH]/[QTVERSION]/[COMPILER]/bin``. For example ```~/Qt/5.9.1/clang_64/bin/qmake```. On Windows, instead of make you can use ```jom.exe``` which installs with msvc compiler.

+ **ccache**: For building with ccache add `QMAKE_CXX='ccache $${QMAKE_CXX}'` to the end of qmake or QtCreator build kit

### Tests

See [tests](tests)
