/* 
 * File:   unitTests.cpp
 * Author: massimo
 *
 * Created on June 19, 2017, 10:43 AM
 */

#include "../deferredThreadScheduler.h"
#include "concurrentLogging.h"
#include <vector>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;
using namespace ::deferredThreadScheduler;
// BEGIN: ignore the warnings listed below when compiled with clang from here
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
////////////////////////////////////////////////////////////////////////////////
TEST(deferredThreadScheduler, deferredThreadSchedulerVersion)
{
  ASSERT_THAT(deferredThreadSchedulerBase::deferredThreadSchedulerVersion(),
              Eq(deferredThreadSchedulerBase::version));
}

TEST(deferredThreadScheduler, test_1)
{
  using threadResultType = int;
  using defaultFun = std::function<threadResultType()>;
  defaultFun intFoo = []() -> threadResultType
  {
    utilities::pclog{} << "[ " << __func__ << " ] "
                       << "intFoo() running................................."
                       << std::endl;
    return 135;
  };

  auto deferredTime = 5s;
  deferredThreadScheduler<threadResultType, defaultFun> dts {"intFoo"};
  dts.registerThread(intFoo);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::NotValid));

  dts.runIn(deferredTime);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));

  while ( static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled) == dts.getThreadState() )
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
}

// passing a thread argument by capturing it by reference
TEST(deferredThreadScheduler, test_2)
{
  std::string threadArg {"Hello World... "};

  using threadResultType = int;
  using defaultFun = std::function<threadResultType()>;
  defaultFun intFoo = [&threadArg] () -> threadResultType
  {
    threadArg += "from intFoo()!!!";
    utilities::pclog{} << "[ " << __func__ << " ] "
                       << "intFoo() running: "
                       << threadArg
                       << "................................."
                       << std::endl;
    return 246;
  };

  auto deferredTime = 3s;
  deferredThreadScheduler<threadResultType, defaultFun> dts {"intFoo"};
  dts.registerThread(intFoo);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::NotValid));

  dts.runIn(deferredTime);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));

  while ( static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled) == dts.getThreadState() )
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
  ASSERT_EQ(threadArg, "Hello World... from intFoo()!!!");
}

// passing a thread argument by capturing it by value
TEST(deferredThreadScheduler, test_2_1)
{
  std::string threadArg {"Hello World... "};

  using threadResultType = int;
  using defaultFun = std::function<threadResultType()>;
  defaultFun intFoo = [threadArg] () mutable -> threadResultType
  {
    threadArg += "from intFoo()!!!";
    utilities::pclog{} << "[ " << __func__ << " ] "
                       << "intFoo() running: "
                       << threadArg
                       << "................................."
                       << std::endl;
    return 357;
  };

  auto deferredTime = 4s;
  deferredThreadScheduler<threadResultType, defaultFun> dts {"intFoo"};
  dts.registerThread(intFoo);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::NotValid));

  dts.runIn(deferredTime);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));

  while ( static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled) == dts.getThreadState() )
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
  ASSERT_EQ(threadArg, "Hello World... ");
}

using threadGlobalResultType = int;
threadGlobalResultType intFooGlobal();
threadGlobalResultType intFooGlobal()
{
  utilities::pclog{} << "[ " << __func__ << " ] "
                     << "intFooGlobal() running................................."
                     << std::endl;
  return 862;
};

TEST(deferredThreadScheduler, test_3)
{
  using threadResultType = threadGlobalResultType;
  auto deferredTime = 1s;
  deferredThreadScheduler<threadResultType, threadResultType()> dts {"intFooGlobal"};
  dts.registerThread(intFooGlobal);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::NotValid));

  dts.runIn(deferredTime);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));

  while ( static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled) == dts.getThreadState() )
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
}

TEST(deferredThreadScheduler, test_4)
{
  using threadResultType = threadGlobalResultType;
  auto deferredTime = 60s;
  deferredThreadScheduler<threadResultType, threadResultType()> dts {"intFooGlobal"};
  dts.registerThread(intFooGlobal);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::NotValid));

  dts.runIn(deferredTime);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));

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
            << " ] to run...";

  // cancel the thread after 7 seconds; the test should take a bit more than
  // 7 seconds, not 60 seconds as requested at the declaration of dts
  std::this_thread::sleep_for(7s);
  ASSERT_EQ(dts.getThreadState(),
          static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));

  dts.cancelThread();

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Cancelled));
  std::cout << " Cancelled."
            << std::endl;
}

// schedule 2 threads at the same time and tun them
TEST(deferredThreadScheduler, test_5)
{
  using threadResultType = int;
  using defaultFun = std::function<threadResultType()>;
  auto deferredTime = 3s;
  int i{1};

  auto dtsPtr_1 = std::make_shared<deferredThreadScheduler<threadResultType, defaultFun>>("intFoo");
  dtsPtr_1.get()->registerThread([i]() -> threadResultType
                              {
                                utilities::pclog{} << "[ " << __func__ << " ] "
                                          << i
                                          << ": intFoo() #1 running: "
                                          << "................................."
                                          << std::endl;
                                return 741;
                              });
  ++i;
  auto dtsPtr_2 = std::make_shared<deferredThreadScheduler<threadResultType, defaultFun>>("intFoo");
  dtsPtr_2.get()->registerThread([i]() -> threadResultType
                              {
                                utilities::pclog{} << "[ " << __func__ << " ] "
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
}

// schedule many threads at the same time and run them
TEST(deferredThreadScheduler, test_6)
{
  using threadResultType = int;
  using defaultFun = std::function<threadResultType()>;
  using dtsSharedPtr = std::shared_ptr<deferredThreadScheduler<threadResultType, defaultFun>>;
  std::vector<dtsSharedPtr> v {};
  auto deferredTime = 3s;

  for(int i = 1; i <= 50; ++i)
  {
    auto dtsPtr = makeSharedDeferredThreadScheduler<threadResultType, defaultFun>("intFoo");

    dtsPtr.get()->registerThread([i]() -> threadResultType
                                 {
                                   utilities::pclog{} << "[ " << __func__ << " ] "
                                             << i
                                             << ": intFoo() running: "
                                             << "................................."
                                             << std::endl;
                                   return 154;
                                 });
    v.push_back(dtsPtr);

    ASSERT_EQ(dtsPtr.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::NotValid));
  }
  for (auto& dts : v)
  {
    dts.get()->runIn(deferredTime);

    ASSERT_EQ(dts.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));
  }
  for (auto& dts : v)
  {
    auto [threadState, threadResult] = dts.get()->wait();
    ASSERT_EQ(154, threadResult);
    ASSERT_EQ(dts.get()->getThreadState(),
              threadState);
    ASSERT_EQ(dts.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
  }
}

// schedule many threads at the same time and cancel them
TEST(deferredThreadScheduler, test_7)
{
  using threadResultType = int;
  using defaultFun = std::function<threadResultType()>;
  using dtsSharedPtr = std::shared_ptr<deferredThreadScheduler<threadResultType, defaultFun>>;
  std::vector<dtsSharedPtr> v {};
  auto deferredTime = 60s;

  for(int i = 1; i <= 10'000; ++i)
  {
    auto dtsPtr = makeSharedDeferredThreadScheduler<threadResultType, defaultFun>("intFoo");
    dtsPtr.get()->registerThread([i]() -> threadResultType
                              {
                                utilities::pclog{} << "[ " << __func__ << " ] "
                                          << i
                                          << ": intFoo() running: "
                                          << "................................."
                                          << std::endl;
                                return 0;
                              });
    v.push_back(dtsPtr);

    ASSERT_EQ(dtsPtr.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::NotValid));
  }
  for (auto& dts : v)
  {
    dts.get()->runIn(deferredTime);

    ASSERT_EQ(dts.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));
  }
  // cancel the thread after 5 seconds; the test should take a bit more than
  // 5 seconds, not 60 seconds as requested at the declaration of dts
  sleep(5);
  for (auto& dts : v)
  {
    dts.get()->cancelThread();

    ASSERT_EQ(dts.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::Cancelled));
  }
}

////////////////////////////////////////////////////////////////////////////////
#pragma clang diagnostic pop
// END: ignore the warnings when compiled with clang up to here
