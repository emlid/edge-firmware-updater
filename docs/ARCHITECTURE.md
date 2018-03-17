# Architecture
## Technologies
* C++14
* Qt 5.9.1

## Module architecture
### Overview
The project contains several modules. Each module:

* has a **public interface** (Classes with ``I`` as prefix) and a **private interface** (classes with ``Impl`` as postfix). 
* interacts with another one via **public interface**.
* provides **factory functions** (Starts with ``make`` prefix) or **factory classes**
* builds into **static library**

### Explanation
* Each module has two interfaces for simplify the testing process. We can easily create stubs and mocks and feed them into the class we test.
* Building modules as static libraries improves performance of tests build. We compile each module once and use it in main application and tests.

### Brief desription
* ```devlib``` - implements **API** for cross-platform interaction with devices.
* ```util``` - contains utilities for **flashing**, **computing crc**, **polling**, etc.
* ```rpi``` - cross-platform implementation of **rpiboot**.
* ```edge``` - **high-level interface** for **Emlid Edge** device. Encapsulate ```rpi``` and ```util```.
* ```main``` - main CLI application. Implements interprocess interaction via ```QtRemoteObjects``` (Sockets).
