# Edge Firmware Updater
> Flash firmare images

***
## Overview
This application was designed for flashing usb devices. General purpose - raspberry device (**Emlid Edge**). 
Firmware updater generally runs as subprocess of any applications. Interprocess communication realized via **QtRemoteObjects**.
For use Firmware Updater from your project you should copy ```src/shared``` path.

### General functions
+ find Edge device in the system
+ boot Edge as mass storage (via rpiboot)
+ get the verision of current firmware
+ flash image
+ validate writing (via CRC)

## Supported Operating Systems
- Linux (most distros)
- Microsoft Windows 7 and later

## Build

### Native Builds
* **Linux**: 64 bit, gcc compiler
* **Windows**: 32 bit, [Visual Studio 2015 compiler](http://www.visualstudio.com/downloads/download-visual-studio-vs#d-express-windows-desktop)
### Qt
* Download the [Qt installer](http://www.qt.io/download-open-source)
    * Make sure to install Qt version **5.9.1 with QtRemoteObjects**. 
    * **Linux**: Set the downloaded file to executable using:`chmod +x`.
### Additional packages
- **Linux**: ```sudo apt-get install libusb-1.0-0-dev libblkid-dev libudev-dev```
