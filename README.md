# deferred-thread-scheduler
A C++17 implementation of a deferred thread scheduler

## Requirements

`cmake` is used to compile the sources.
The default compiler used is `clang++-4.0`.
The cmake file compiles with `-std=c++1z`.

The unit tests are implemented in `googletest`: be sure you have installed `googletest` to compile.


## Install and Run Unit Tests

```bash
$ git clone git@github.com:massimo-marino/deferred-thread-scheduler.git
$ cd deferred-thread-scheduler/
$ mkdir build
$ cd build
$ cmake ..
$ make
$ cd src/unitTests
$ ./unitTests
```
The unit tests provide examples of usage of active classes.


## The Deferred Thread Scheduler

A template class named `deferredThreadScheduler` that manages the execution of deferred tasks.

This class allows the creation of instances that:

- take: a string that identifies a thread; a callable object that identifies the function to be run in the thread; a deferred time in seconds from the time of instance creation when the thread must be started;

- provide a method for cancelling a scheduled thread; the cancel request is to be ignored if the thread has already started.

