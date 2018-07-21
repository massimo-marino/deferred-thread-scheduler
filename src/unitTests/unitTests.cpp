/* 
 * File:   unitTests.cpp
 * Author: massimo
 *
 * Created on June 19, 2017, 10:43 AM
 */

#include "../deferredThreadScheduler.h"
#include "concurrentLogging.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
////////////////////////////////////////////////////////////////////////////////
using namespace ::testing;
using namespace ::utilities;
using namespace ::deferredThreadScheduler;
// BEGIN: ignore the warnings listed below when compiled with clang from here
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
////////////////////////////////////////////////////////////////////////////////
TEST(deferredThreadScheduler, deferredThreadSchedulerVersion)
{
  ASSERT_THAT(deferredThreadSchedulerBase::deferredThreadSchedulerVersion(),
              Eq(deferredThreadSchedulerBase::version));
}

TEST(deferredThreadScheduler, test_1)
{
  using threadResultType = int;
  using threadFun = std::function<threadResultType()>;
  threadFun intFoo = []() noexcept(false) -> threadResultType
  {
    pclog{} << "[ " << __func__ << " ] "
            << "intFoo() running................................."
            << std::endl;
    return 135;
  };

  deferredThreadScheduler<threadResultType, threadFun> dts {"intFoo"};
  dts.registerThread(intFoo);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Registered));
  ASSERT_EQ(dts.isRegistered(), true);

  auto deferredTime {5s};
  dts.runIn(deferredTime);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));
  ASSERT_EQ(dts.isScheduled(), true);

  while ( dts.isScheduled() )
  {
    usleep(1'000);
  }

  std::cout << "[ " << __func__ << " ] "
            << "Thread name: "
            << dts.getThreadName()
            << std::endl;
  std::cout << "[ " << __func__ << " ] "
            << "waiting for thread "
            << dts.getThreadName()
            << " [ id 0x"
            << std::hex << dts.getThreadId() << std::dec
            << " ] to run..."
            << std::endl;

  auto [threadState, threadResult] = dts.wait();

  ASSERT_EQ(135, threadResult);
  ASSERT_EQ(dts.getThreadState(),
            threadState);
  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
  ASSERT_EQ(dts.isRun(), true);
  ASSERT_EQ(dts.isRun(threadState), true);
}

// passing a thread argument by capturing it by reference
TEST(deferredThreadScheduler, test_2)
{
  std::string threadArg {"Hello World... "};

  using threadResultType = int;
  using threadFun = std::function<threadResultType()>;
  threadFun intFoo = [&threadArg]() noexcept(false) -> threadResultType
  {
    threadArg += "from intFoo()!!!";
    pclog{} << "[ " << __func__ << " ] "
            << "intFoo() running: "
            << threadArg
            << "................................."
            << std::endl;
    return 246;
  };

  deferredThreadScheduler<threadResultType, threadFun> dts {"intFoo"};
  dts.registerThread(intFoo);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Registered));
  ASSERT_EQ(dts.isRegistered(), true);

  auto deferredTime {3s};
  dts.runIn(deferredTime);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));
  ASSERT_EQ(dts.isScheduled(), true);

  while ( dts.isScheduled() )
  {
    usleep(1'000);
  }

  std::cout << "[ " << __func__ << " ] "
            << "Thread name: "
            << dts.getThreadName()
            << std::endl;
  std::cout << "[ " << __func__ << " ] "
            << "waiting for thread "
            << dts.getThreadName()
            << " [ id 0x"
            << std::hex << dts.getThreadId() << std::dec
            << " ] to run..."
            << std::endl;

  auto [threadState, threadResult] = dts.wait();

  ASSERT_EQ(246, threadResult);
  ASSERT_EQ(dts.getThreadState(),
            threadState);
  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
  ASSERT_EQ(dts.isRun(), true);
  ASSERT_EQ(dts.isRun(threadState), true);
  ASSERT_EQ(threadArg, "Hello World... from intFoo()!!!");
}

// passing a thread argument by capturing it by value
TEST(deferredThreadScheduler, test_2_1)
{
  std::string threadArg {"Hello World... "};

  using threadResultType = int;
  using threadFun = std::function<threadResultType()>;
  threadFun intFoo = [threadArg]() mutable noexcept(false) -> threadResultType
  {
    threadArg += "from intFoo()!!!";
    pclog{} << "[ " << __func__ << " ] "
            << "intFoo() running: "
            << threadArg
            << "................................."
            << std::endl;
    return 357;
  };

  deferredThreadScheduler<threadResultType, threadFun> dts {"intFoo"};
  dts.registerThread(intFoo);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Registered));
  ASSERT_EQ(dts.isRegistered(), true);

  auto deferredTime {4s};
  dts.runIn(deferredTime);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));
  ASSERT_EQ(dts.isScheduled(), true);

  while ( dts.isScheduled() )
  {
    usleep(1'000);
  }

  std::cout << "[ " << __func__ << " ] "
            << "Thread name: "
            << dts.getThreadName()
            << std::endl;
  std::cout << "[ " << __func__ << " ] "
            << "waiting for thread "
            << dts.getThreadName()
            << " [ id 0x"
            << std::hex << dts.getThreadId() << std::dec
            << " ] to run..."
            << std::endl;

  auto [threadState, threadResult] = dts.wait();

  ASSERT_EQ(357, threadResult);
  ASSERT_EQ(dts.getThreadState(),
            threadState);
  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
  ASSERT_EQ(dts.isRun(), true);
  ASSERT_EQ(dts.isRun(threadState), true);
  ASSERT_EQ(threadArg, "Hello World... ");
}

using threadGlobalResultType = int;
threadGlobalResultType intFooGlobal() noexcept(false);
threadGlobalResultType intFooGlobal() noexcept(false)
{
  pclog{} << "[ " << __func__ << " ] "
          << "intFooGlobal() running................................."
          << std::endl;
  return 862;
};

TEST(deferredThreadScheduler, test_3)
{
  using threadResultType = threadGlobalResultType;
  auto deferredTime {1'000'000us};
  deferredThreadScheduler<threadResultType, threadResultType()> dts {"intFooGlobal"};
  dts.registerThread(intFooGlobal);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Registered));
  ASSERT_EQ(dts.isRegistered(), true);

  dts.runIn(deferredTime);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));
  ASSERT_EQ(dts.isScheduled(), true);

  while ( dts.isScheduled() )
  {
    usleep(1'000);
  }

  std::cout << "[ " << __func__ << " ] "
            << "Thread name: "
            << dts.getThreadName()
            << std::endl;
  std::cout << "[ " << __func__ << " ] "
            << "waiting for thread "
            << dts.getThreadName()
            << " [ id 0x"
            << std::hex << dts.getThreadId() << std::dec
            << " ] to run..."
            << std::endl;

  auto [threadState, threadResult] = dts.wait();

  ASSERT_EQ(862, threadResult);
  ASSERT_EQ(dts.getThreadState(),
            threadState);
  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
  ASSERT_EQ(dts.isRun(), true);
  ASSERT_EQ(dts.isRun(threadState), true);
}

TEST(deferredThreadScheduler, test_4)
{
  using threadResultType = threadGlobalResultType;
  auto deferredTime {60s};
  deferredThreadScheduler<threadResultType, threadResultType()> dts {"intFooGlobal"};
  dts.registerThread(intFooGlobal);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Registered));
  ASSERT_EQ(dts.isRegistered(), true);

  dts.runIn(deferredTime);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));
  ASSERT_EQ(dts.isScheduled(), true);

  usleep(5'000);
  std::cout << "[ " << __func__ << " ] "
            << "Thread name: "
            << dts.getThreadName()
            << std::endl;
  std::cout << "[ " << __func__ << " ] "
            << "waiting for thread "
            << dts.getThreadName()
            << " [ id 0x"
            << std::hex << dts.getThreadId() << std::dec
            << " ] to run ";

  // cancel the thread after 7 seconds; the test should take a bit more than
  // 7 seconds, not 60 seconds as requested at the declaration of dts
  for (int i {1}; i <= 70; ++i)
  {
    auto [threadState, threadResult] = dts.wait_for(100ms);
    if ( dts.isRun(threadState) )
    {
      break;
    }
    std::cout << "." << std::flush;
  }
  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));
  ASSERT_EQ(dts.isScheduled(), true);

  auto result = dts.cancelThread();

  ASSERT_EQ(true, result);
  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Canceled));
  ASSERT_EQ(dts.isCanceled(), true);
  std::cout << " Thread canceled."
            << std::endl;
}

// schedule 2 threads at the same time and run them
TEST(deferredThreadScheduler, test_5)
{
  using threadResultType = int;
  using threadFun = std::function<threadResultType()>;
  auto deferredTime {3'000'000'000ns};
  int i {1};

  auto dtsPtr_1 = std::make_shared<deferredThreadScheduler<threadResultType, threadFun>>("intFoo");
  dtsPtr_1.get()->registerThread([i]() noexcept(false) -> threadResultType
                              {
                                pclog{} << "[ " << __func__ << " ] "
                                        << i
                                        << ": intFoo() #1 running: "
                                        << "................................."
                                        << std::endl;
                                return 741;
                              });
  ++i;
  auto dtsPtr_2 = std::make_shared<deferredThreadScheduler<threadResultType, threadFun>>("intFoo");
  dtsPtr_2.get()->registerThread([i]() noexcept(false) -> threadResultType
                              {
                                pclog{} << "[ " << __func__ << " ] "
                                        << i
                                        << ": intFoo() #2 running: "
                                        << "................................."
                                        << std::endl;
                                return 963;
                              });

  dtsPtr_1.get()->runIn(deferredTime);
  dtsPtr_2.get()->runIn(deferredTime);

  auto [threadState_1, threadResult_1] = dtsPtr_1.get()->wait();
  auto [threadState_2, threadResult_2] = dtsPtr_2.get()->wait();

  std::cout << "[ " << __func__ << " ] "
          << "thread 1 "
          << dtsPtr_1.get()->getThreadName()
          << " [ id 0x"
          << std::hex << dtsPtr_1.get()->getThreadId() << std::dec
          << " ]"
          << std::endl;
  std::cout << "[ " << __func__ << " ] "
          << "thread 2 "
          << dtsPtr_2.get()->getThreadName()
          << " [ id 0x"
          << std::hex << dtsPtr_2.get()->getThreadId() << std::dec
          << " ]"
          << std::endl;

  ASSERT_EQ(741, threadResult_1);
  ASSERT_EQ(963, threadResult_2);
  ASSERT_EQ(dtsPtr_1.get()->getThreadState(),
            threadState_1);
  ASSERT_EQ(dtsPtr_2.get()->getThreadState(),
            threadState_2);
  ASSERT_EQ(dtsPtr_1.get()->getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
  ASSERT_EQ(dtsPtr_2.get()->getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
  ASSERT_EQ(dtsPtr_1.get()->isRun(), true);
  ASSERT_EQ(dtsPtr_2.get()->isRun(), true);
  ASSERT_EQ(dtsPtr_1.get()->isRun(threadState_1), true);
  ASSERT_EQ(dtsPtr_2.get()->isRun(threadState_2), true);
}

// schedule many threads at the same time and run them
TEST(deferredThreadScheduler, test_6)
{
  using threadResultType = int;
  using threadFun = std::function<threadResultType()>;
  using dtsSharedPtr = std::shared_ptr<deferredThreadScheduler<threadResultType, threadFun>>;
  std::vector<dtsSharedPtr> v {};
  auto deferredTime {3s};
  const int numThreads {10'000};

  for (int i {1}; i <= numThreads; ++i)
  {
    auto dtsPtr = makeSharedDeferredThreadScheduler<threadResultType, threadFun>("intFoo");
    dtsPtr.get()->registerThread([]() noexcept(false) -> threadResultType
                                 {
                                   return 42;
                                 });
    v.push_back(dtsPtr);

    ASSERT_EQ(dtsPtr.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::Registered));
    ASSERT_EQ(dtsPtr.get()->isRegistered(), true);
  }
  ASSERT_EQ(numThreads, v.size());
  for (auto& dts : v)
  {
    dts.get()->runIn(deferredTime);

    ASSERT_EQ(dts.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));
    ASSERT_EQ(dts.get()->isScheduled(), true);
  }
  for (auto& dts : v)
  {
    auto [threadState, threadResult] = dts.get()->wait();
    ASSERT_EQ(42, threadResult);
    ASSERT_EQ(dts.get()->getThreadState(),
              threadState);
    ASSERT_EQ(dts.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
    ASSERT_EQ(dts.get()->isRun(), true);
    ASSERT_EQ(dts.get()->isRun(threadState), true);
  }
}

// schedule many threads with a deferred time of 0 seconds, and run them
TEST(deferredThreadScheduler, test_6_1)
{
  using threadResultType = int;
  using threadFun = std::function<threadResultType()>;
  using dtsSharedPtr = std::shared_ptr<deferredThreadScheduler<threadResultType, threadFun>>;
  std::vector<dtsSharedPtr> v {};
  auto deferredTime {0s};
  const int numThreads {10'000};

  for (int i {1}; i <= numThreads; ++i)
  {
    auto dtsPtr = makeSharedDeferredThreadScheduler<threadResultType, threadFun>("intFoo");
    dtsPtr.get()->registerThread([]() noexcept(false) -> threadResultType
                                 {
                                   return 42;
                                 });
    v.push_back(dtsPtr);

    ASSERT_EQ(dtsPtr.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::Registered));
    ASSERT_EQ(dtsPtr.get()->isRegistered(), true);
  }
  ASSERT_EQ(numThreads, v.size());
  for (auto& dts : v)
  {
    // since the deferred time is 0 seconds we should expect the thread status
    // as either: scheduled, running, or run
    dts.get()->runIn(deferredTime);

    ASSERT_TRUE(dts.get()->getThreadState() == static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled) ||
                dts.get()->getThreadState() == static_cast<int>(deferredThreadSchedulerBase::threadState::Running) ||
                dts.get()->getThreadState() == static_cast<int>(deferredThreadSchedulerBase::threadState::Run) );

    ASSERT_TRUE(dts.get()->isScheduled() == true ||
                dts.get()->isRunning() == true ||
                dts.get()->isRun() == true );
  }
  for (auto& dts : v)
  {
    auto [threadState, threadResult] = dts.get()->wait();
    ASSERT_EQ(42, threadResult);
    ASSERT_EQ(dts.get()->getThreadState(),
              threadState);
    ASSERT_EQ(dts.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
    ASSERT_EQ(dts.get()->isRun(), true);
    ASSERT_EQ(dts.get()->isRun(threadState), true);
  }
}

// schedule many threads at the same time and cancel them
TEST(deferredThreadScheduler, test_7)
{
  using threadResultType = int;
  using threadFun = std::function<threadResultType()>;
  using dtsSharedPtr = std::shared_ptr<deferredThreadScheduler<threadResultType, threadFun>>;
  std::vector<dtsSharedPtr> v {};
  auto deferredTime {60s};
  const int numThreads {10'000};

  for (int i {1}; i <= numThreads; ++i)
  {
    auto dtsPtr = makeSharedDeferredThreadScheduler<threadResultType, threadFun>("intFoo");
    dtsPtr.get()->registerThread([i]() noexcept(false) -> threadResultType
                              {
                                pclog{} << "[ " << __func__ << " ] "
                                        << i
                                        << ": intFoo() running: "
                                        << "................................."
                                        << std::endl;
                                return 42;
                              });
    v.push_back(dtsPtr);

    ASSERT_EQ(dtsPtr.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::Registered));
    ASSERT_EQ(dtsPtr.get()->isRegistered(), true);
  }
  ASSERT_EQ(numThreads, v.size());
  for (auto& dts : v)
  {
    dts.get()->runIn(deferredTime);

    ASSERT_EQ(dts.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));
    ASSERT_EQ(dts.get()->isScheduled(), true);
  }
  // cancel the thread after 5 seconds; the test should take a bit more than
  // 5 seconds, not 60 seconds as requested at the declaration of dts
  sleep(5);
  bool result {};
  for (auto& dts : v)
  {
    result = dts.get()->cancelThread();

    ASSERT_EQ(true, result);
    ASSERT_EQ(dts.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::Canceled));
    ASSERT_EQ(dts.get()->isCanceled(), true);
  }
}

TEST(deferredThreadScheduler, test_8)
{
  using threadResultType = std::string;
  using threadFun = std::function<threadResultType(const std::string&, const std::string&)>;

  std::string s1 {"Hello "};
  std::string s2 {"World!!!"};

  threadFun concatStrings = [](const std::string& str1,
                               const std::string& str2) noexcept(false) -> threadResultType const
                            {
                              return str1 + str2;
                            };

  deferredThreadScheduler<threadResultType, threadFun> dts {"concatStrings"};
  dts.registerThread(concatStrings, s1, s2);

  ASSERT_EQ(dts.getThreadState(),
          static_cast<int>(deferredThreadSchedulerBase::threadState::Registered));
  ASSERT_EQ(dts.isRegistered(), true);

  auto deferredTime = 3.0;
  dts.runIn(deferredTime);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));
  ASSERT_EQ(dts.isScheduled(), true);

  auto [threadState, threadResult] = dts.wait();

  ASSERT_EQ(threadState,
            dts.getThreadState());
  ASSERT_EQ(threadState,
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
  ASSERT_EQ(dts.isRun(), true);
  ASSERT_EQ(dts.isRun(threadState), true);
  ASSERT_EQ("Hello World!!!",
            threadResult);
}

TEST(deferredThreadScheduler, test_9)
{
  using threadResultType = std::string;
  using threadFun = std::function<threadResultType(const std::string&, const std::string&)>;

  std::string s1 {"Hello "};
  std::string s2 {"World!!!"};

  threadFun concatStrings = [](const std::string& str1,
                               const std::string& str2) noexcept(false) -> threadResultType const
                            {
                              return str1 + str2;
                            };

  deferredThreadScheduler<threadResultType, threadFun> dts {"concatStrings", concatStrings, s1, s2};

  ASSERT_EQ(dts.getThreadState(),
          static_cast<int>(deferredThreadSchedulerBase::threadState::Registered));
  ASSERT_EQ(dts.isRegistered(), true);

  auto deferredTime = 2s;
  dts.runIn(deferredTime);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));
  ASSERT_EQ(dts.isScheduled(), true);

  auto [threadState, threadResult] = dts.wait();

  ASSERT_EQ(threadState,
            dts.getThreadState());
  ASSERT_EQ(threadState,
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
  ASSERT_EQ(dts.isRun(), true);
  ASSERT_EQ(dts.isRun(threadState), true);
  ASSERT_EQ("Hello World!!!",
            threadResult);
}

TEST(deferredThreadScheduler, test_10)
{
  // the result type of the thread function
  using threadResultType = std::string;
  // the thread function type/signature
  using threadFun = std::function<threadResultType(const std::string&, const std::string&)>;

  // the arguments passed to the thread function
  std::string s1 {"Hello "};
  std::string s2 {"World!!!"};

  // the thread function
  threadFun concatStrings = [](const std::string& str1,
                               const std::string& str2) noexcept(false) -> threadResultType const
                            {
                              return str1 + str2;
                            };

  // create an object for the deferred thread scheduler
  deferredThreadScheduler<threadResultType, threadFun> dts {"concatStrings"};
  
  // register the thread, schedule the thread to run in 2s from now, and wait its termination
  auto [threadState, threadResult] = dts.registerThread(concatStrings, s1, s2).runIn(2s).wait();

  ASSERT_EQ(threadState,
            dts.getThreadState());
  ASSERT_EQ(threadState,
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
  ASSERT_EQ(dts.isRun(), true);
  ASSERT_EQ(dts.isRun(threadState), true);
  ASSERT_EQ("Hello World!!!",
            threadResult);
}

// in this test an exception is thrown by the thread function 
TEST(deferredThreadScheduler, test_11)
{
  {
    // the result type of the thread function
    using threadResultType = int;
    // the thread function type/signature
    using threadFun = std::function<threadResultType()>;

    // the thread function
    threadFun wellLoop = []() noexcept(false) -> threadResultType
                         {
                           std::this_thread::sleep_for(1s);
                           throw std::runtime_error("runtime error");
                         };

    // create an object for the deferred thread scheduler
    deferredThreadScheduler<threadResultType, threadFun> dts {"wellLoop"};

    // register the thread, schedule the thread to run in 2s from now
    dts.registerThread(wellLoop).runIn(1s);
    // wait the thread throws the exception
    while ( false == dts.isExceptionThrown() )
    {
      auto [threadState, threadResult] = dts.wait_for();
      std::cout << "." << std::flush;
      std::this_thread::sleep_for(1s);
    }
    ASSERT_EQ(dts.isExceptionThrown(), true);
    if ( dts.isExceptionThrown() )
    {
      std::cout << dts.getExceptionThrownMessage() << std::endl;
    }
  }
}

// in this test an exception is thrown by the thread function 
TEST(deferredThreadScheduler, test_12)
{
  {
    // the result type of the thread function
    using threadResultType = int;
    // the thread function type/signature
    using threadFun = std::function<threadResultType()>;

    // the thread function
    threadFun wellLoop = []() noexcept(false) -> threadResultType
                         {
                           std::this_thread::sleep_for(1s);
                           throw std::runtime_error("runtime error");
                         };

    // create an object for the deferred thread scheduler
    deferredThreadScheduler<threadResultType, threadFun> dts {"wellLoop"};

    // register the thread, schedule the thread to run in 1s from now
    dts.registerThread(wellLoop).runIn(1s);
    // wait the thread throws the exception
    auto [threadState, threadResult] = dts.wait();

    ASSERT_EQ(dts.isExceptionThrown(), true);
    if ( dts.isExceptionThrown() )
    {
      std::cout << dts.getExceptionThrownMessage() << std::endl;
    }
  }
}

// test that the cancellation flag is set at a safe cancellation point so that
// the thread can be terminated safely
TEST(deferredThreadScheduler, test_13)
{
  {
    // the result type of the thread function
    using threadResultType = int;
    // the thread function type/signature
    using threadFun = std::function<threadResultType()>;

    // the thread function
    threadFun wellLoop = []() noexcept(false) -> threadResultType const
                         {
                           std::cout << "RUNNING" << std::endl;
                           while ( true )
                           {
                             std::cout << "*" << std::flush;
                             std::this_thread::sleep_for(1s);
                             // safe cancellation point, use the macro
                             TERMINATE_ON_CANCELLATION(threadResultType)
                           }
                         };

    // create an object for the deferred thread scheduler
    deferredThreadScheduler<threadResultType, threadFun> dts {"wellLoop"};

    // register the thread, schedule the thread to run in 2s from now
    dts.registerThread(wellLoop).runIn(2000ms);
    while ( false == dts.isRunning() )
    {
      std::cout << "." << std::flush;
      std::this_thread::sleep_for(1s);
    }
    ASSERT_EQ(dts.isRunning(), true);
    deferredThreadSchedulerBase::listCancellationFlags(std::cout);
    std::cout << "exiting scope..." << std::flush;
    // dts object is destroyed, and cancellation flag for the thread is set;
    // since the thread checks the flag, it will terminate
  }
  auto [cfSize, cfSet, cfUnset] = deferredThreadSchedulerBase::listCancellationFlags(std::cout);
  ASSERT_EQ(0, cfSize);
  ASSERT_EQ(0, cfSet);
  ASSERT_EQ(0, cfUnset);
}

// schedule many threads with a deferred time of 0 seconds, run them, and leave
// the scope so the dtor is called and the thread's cancellation flags are set
TEST(deferredThreadScheduler, test_14)
{
  const unsigned int maxLoops {25};

  for (unsigned int l {1}; l <= maxLoops; ++l)
  {
    {
      using threadResultType = int;
      using threadFun = std::function<threadResultType()>;
      using dtsSharedPtr = std::shared_ptr<deferredThreadScheduler<threadResultType, threadFun>>;
      std::vector<dtsSharedPtr> v {};
      auto deferredTime {0s};
      const unsigned int numThreads {10'000};

      for (unsigned int i {1}; i <= numThreads; ++i)
      {
        auto dtsPtr = makeSharedDeferredThreadScheduler<threadResultType, threadFun>("intFoo");
        dtsPtr.get()->registerThread([]() noexcept(false) -> threadResultType
                                     {
                                       std::this_thread::sleep_for(std::chrono::milliseconds(750));
                                       // safe cancellation point, use the macro
                                       TERMINATE_ON_CANCELLATION(threadResultType)
                                     } );
        v.push_back(dtsPtr);

        ASSERT_EQ(dtsPtr.get()->getThreadState(),
                  static_cast<int>(deferredThreadSchedulerBase::threadState::Registered));
        ASSERT_EQ(dtsPtr.get()->isRegistered(), true);
      }
      ASSERT_EQ(numThreads, v.size());
      for (auto& dts : v)
      {
        // since the deferred time is 0 seconds we should expect the thread status
        // as either: scheduled, running, or run
        dts.get()->runIn(deferredTime);

        ASSERT_TRUE(dts.get()->getThreadState() == static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled) ||
                    dts.get()->getThreadState() == static_cast<int>(deferredThreadSchedulerBase::threadState::Running) ||
                    dts.get()->getThreadState() == static_cast<int>(deferredThreadSchedulerBase::threadState::Run) );

        ASSERT_TRUE(dts.get()->isScheduled() == true ||
                    dts.get()->isRunning() == true ||
                    dts.get()->isRun() == true );
      }
      std::cout << l
                << "/"
                << maxLoops
                << ": exiting scope..." << std::flush;
      // all dts objects are destroyed, and cancellation flags for the threads that
      // are not yet terminated are set; since the threads check the flag, they
      // will terminate
    }
    auto [cfSize, cfSet, cfUnset] = deferredThreadSchedulerBase::listCancellationFlags(std::cout);
    ASSERT_EQ(0, cfSize);
    ASSERT_EQ(0, cfSet);
    ASSERT_EQ(0, cfUnset);
  }
}

TEST(deferredThreadScheduler,last_test)
{
  auto [cfSize, cfSet, cfUnset] = deferredThreadSchedulerBase::listCancellationFlags(std::cout);
  ASSERT_EQ(0, cfSize);
  ASSERT_EQ(0, cfSet);
  ASSERT_EQ(0, cfUnset);
}
////////////////////////////////////////////////////////////////////////////////
#pragma clang diagnostic pop
// END: ignore the warnings when compiled with clang up to here
