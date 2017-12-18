# Project structure
***
    
## Firmware Upgrader Watcher
  + this class is observer
  + used for execute **subtasks**, receive and emit signals from another process
  + give an API for interprocess communication via **QtRemoteObjects**
  
## Subtasks
  + an operations which can be executed in another thread (cancelled and stopped)
  
## devapi
  + contains all modules related with device scanning
  + contains **OS specific** implementation of **storage device manager** and **storage device** classes
