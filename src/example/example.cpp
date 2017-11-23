/* 
 * File:   example.cpp
 * Author: massimo
 * 
 * Created on November 17, 2017, 11:57 AM
 */
#include "../deferredThreadScheduler.h"
#include <iostream>

using namespace std::chrono_literals;
////////////////////////////////////////////////////////////////////////////////
auto main() -> int
{
  std::clog << "\n[" << __func__ << "] "
            << "Deferred Thread Scheduler Example STARTED\n";

  // the result type of the thread function
  using threadResultType = std::string;
  // the thread function type/signature
  using threadFun = std::function<threadResultType(const std::string&, const std::string&)>;

  // the arguments passed to the thread function
  std::string s1 {"Hello "};
  std::string s2 {"World!!!"};

  // the thread function
  threadFun concatStrings = [](const std::string& str1, const std::string& str2) -> threadResultType
                            {
                              return str1 + str2;
                            };

  // create an object for the deferred thread scheduler and register the thread
  deferredThreadScheduler::deferredThreadScheduler<threadResultType, threadFun> dts {"concatStrings", concatStrings, s1, s2};

  if ( static_cast<int>(deferredThreadScheduler::deferredThreadSchedulerBase::threadState::Registered) == dts.getThreadState() )
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
  auto deferredTime = 4s;
  // schedule the thread to run in deferredTime seconds from now
  dts.runIn(deferredTime);

  if ( static_cast<int>(deferredThreadScheduler::deferredThreadSchedulerBase::threadState::Scheduled) == dts.getThreadState() )
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
  auto [threadState, threadResult] = dts.wait();

  if ( threadState == static_cast<int>(deferredThreadScheduler::deferredThreadSchedulerBase::threadState::Run) )
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

  std::clog << "[" << __func__ << "] "
            << "Deferred Thread Scheduler Example ENDED\n"
            << std::endl;
  
  return 0;
}  // main
