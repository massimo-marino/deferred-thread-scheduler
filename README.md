# deferred-thread-scheduler
A C++17 implementation of a deferred thread scheduler

## Requirements

`cmake` is used to compile the sources.

The default compiler used is `clang++-5.0`.

The cmake files compile with `-std=c++17`.

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
The unit tests provide examples of usage.


## The Deferred Thread Scheduler

A template class named `deferredThreadScheduler` that manages the execution of deferred tasks.

This class allows the creation of instances that:

- take: a string that identifies a thread; a callable object that identifies the function to be run as a thread; a deferred time in seconds when the thread must be started counted from the time the method `runIn()` is called;

- provide a method for cancelling a scheduled thread; the cancel request is to be ignored if the thread has already started.


## Example

How to install and run the example:

```bash
$ git clone git@github.com:massimo-marino/deferred-thread-scheduler.git
$ cd deferred-thread-scheduler/
$ mkdir build
$ cd build
$ cmake ..
$ make
$ cd src/example
$ ./example

[main] Deferred Thread Scheduler Example STARTED
[main] Registered: OK
[main] Scheduled: OK
[main] Run: OK
[main] Thread result: 'Hello World!!!'
[main] Deferred Thread Scheduler Example ENDED
```

The example source code commented is here below to show how to use the class.


```C++
/* 
 * File:   example.cpp
 * Author: massimo
 * 
 * Created on November 17, 2017, 11:57 AM
 */

#include "../deferredThreadScheduler.h"
////////////////////////////////////////////////////////////////////////////////
auto main() -> int
{
  using namespace std::chrono_literals;
  using namespace deferredThreadScheduler;

  std::cout << "\n[" << __func__ << "] "
            << "Deferred Thread Scheduler Example STARTED\n";

  // the result type of the thread function
  using threadResultType = std::string;
  // the thread function type/signature
  using threadFun = std::function<threadResultType(const std::string&, const std::string&)>;

  // the arguments passed to the thread function
  std::string s1 {"Hello "};
  std::string s2 {"World!!!"};

  // the thread function
  threadFun concatStrings = [](const auto& str1, const auto& str2)
                            {
                              return str1 + str2;
                            };

  // create an object for the deferred thread scheduler and register the thread function
  deferredThreadScheduler<threadResultType, threadFun> dts {"concatStrings", concatStrings, s1, s2};

  if ( dts.isRegistered() )
  {
    std::cout << "[" << __func__ << "] "
              << "Registered: OK\n";
  }
  else
  {
    std::cout << "[" << __func__ << "] "
              << "Registered: NOT OK\n";
  }

  // the deferred time in seconds
  auto deferredTime {4s};
  // schedule the thread to run in deferredTime seconds from now
  dts.runIn(deferredTime);

  if ( dts.isScheduled() )
  {
    std::cout << "[" << __func__ << "] "
              << "Scheduled: OK\n";
  }
  else
  {
    std::cout << "[" << __func__ << "] "
              << "Scheduled: NOT OK\n";
  }

  // wait here the end of the thread
  auto [threadState, threadResult] = dts.wait_for(3950ms);
  // dont loop forever, just some more time after the time-out
  for(auto i {1}; i <= 200 && false == dts.isRun(threadState); ++i)
  {
    std::tie(threadState, threadResult) = dts.wait_for(1ms);
  }

  if ( dts.isRun(threadState) )
  {
    std::cout << "[" << __func__ << "] "
              << "Run: OK\n";
  }
  else
  {
    std::cout << "[" << __func__ << "] "
              << "Run: NOT OK\n";
  }

  std::cout << "[" << __func__ << "] "
            << "Thread result: '"
            << threadResult
            << "'\n";

  std::cout << "[" << __func__ << "] "
            << "Deferred Thread Scheduler Example ENDED\n"
            << std::endl;

  return 0;
}  // main

```
