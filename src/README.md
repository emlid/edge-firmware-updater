# Project structure
***
## Firmware Upgrader
  + general class which run all steps of firmware upgrading
  + contains:
    - **rpiboot** module from devapi
    - **storage device manager** module for managing all usb device in the system (searching, unmounting etc)
    - **flasher** module for flashing devices (per-block writing)
    - **checksum calculator** module for checking correctness of flashing
    
## Firmware Upgrader Watcher
  + this class is observer of Firmware Upgrader component
  + used for giving API for interprocess communication via QtRemoteObjects
  
## devapi
  + contains all modules related with device scanning
  + contains **OS specific** implementation of **storage device manager** and **storage device classes**
