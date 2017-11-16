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
  using defaultFun = std::function<void()>;
  defaultFun voidFoo = []() -> void
  {
    utilities::pclog{} << "[ " << __func__ << " ] "
                       << "voidFoo() running................................."
                       << std::endl;
  };

  auto deferredTime = 5s;
  deferredThreadScheduler<defaultFun> dts {"voidFoo"};
  dts.registerThread(voidFoo);

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
            << dts.getThreadName() << std::endl;
  std::cout << "[ " << __func__ << " ] "
            << "waiting for thread "
            << dts.getThreadName()
            << " [ id 0x"
            << std::hex << dts.getThreadId() << std::dec
            << " ] to run..." << std::endl;
  dts.wait();

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
}

// passing a thread argument by capturing it by reference
TEST(deferredThreadScheduler, test_2)
{
  std::string threadArg {"Hello World... "};

  using defaultFun = std::function<void()>;
  defaultFun voidFoo = [&threadArg] () -> void
  {
    threadArg += "from voidFoo()!!!";
    utilities::pclog{} << "[ " << __func__ << " ] "
                       << "voidFoo() running: "
                       << threadArg
                       << "................................."
                       << std::endl;
  };

  auto deferredTime = 3s;
  deferredThreadScheduler<defaultFun> dts {"voidFoo"};
  dts.registerThread(voidFoo);

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
            << dts.getThreadName() << std::endl;
  std::cout << "[ " << __func__ << " ] "
            << "waiting for thread "
            << dts.getThreadName()
            << " [ id 0x"
            << std::hex << dts.getThreadId() << std::dec
            << " ] to run..." << std::endl;
  dts.wait();

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
  ASSERT_EQ(threadArg, "Hello World... from voidFoo()!!!");
}

// passing a thread argument by capturing it by value
TEST(deferredThreadScheduler, test_2_1)
{
  std::string threadArg {"Hello World... "};

  using defaultFun = std::function<void()>;
  defaultFun voidFoo = [threadArg] () mutable -> void
  {
    threadArg += "from voidFoo()!!!";
    utilities::pclog{} << "[ " << __func__ << " ] "
                       << "voidFoo() running: "
                       << threadArg
                       << "................................."
                       << std::endl;
  };

  auto deferredTime = 4s;
  deferredThreadScheduler<defaultFun> dts {"voidFoo"};
  dts.registerThread(voidFoo);

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
            << dts.getThreadName() << std::endl;
  std::cout << "[ " << __func__ << " ] "
            << "waiting for thread "
            << dts.getThreadName()
            << " [ id 0x"
            << std::hex << dts.getThreadId() << std::dec
            << " ] to run..." << std::endl;
  dts.wait();

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
  ASSERT_EQ(threadArg, "Hello World... ");
}

void voidFooGlobal();
void voidFooGlobal()
{
  utilities::pclog{} << "[ " << __func__ << " ] "
                     << "voidFooGlobal() running................................."
                     << std::endl;
};

TEST(deferredThreadScheduler, test_3)
{
  auto deferredTime = 1s;
  deferredThreadScheduler<void()> dts {"voidFooGlobal"};
  dts.registerThread(voidFooGlobal);

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
            << dts.getThreadName() << std::endl;
  std::cout << "[ " << __func__ << " ] "
            << "waiting for thread "
            << dts.getThreadName()
            << " [ id 0x"
            << std::hex << dts.getThreadId() << std::dec
            << " ] to run..." << std::endl;
  dts.wait();

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
}

TEST(deferredThreadScheduler, test_4)
{
  auto deferredTime = 60s;
  deferredThreadScheduler<void()> dts {"voidFooGlobal"};
  dts.registerThread(voidFooGlobal);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::NotValid));

  dts.runIn(deferredTime);

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));

  usleep(5'000);
  std::cout << "[ " << __func__ << " ] "
            << "Thread name: "
            << dts.getThreadName() << std::endl;
  std::cout << "[ " << __func__ << " ] "
            << "waiting for thread "
            << dts.getThreadName()
            << " [ id 0x"
            << std::hex << dts.getThreadId() << std::dec
            << " ] to run..." << std::endl;

  // cancel the thread after 7 seconds; the test should take a bit more than
  // 7 seconds, not 60 seconds as requested at the declaration of dts
  std::this_thread::sleep_for(7s);
  ASSERT_EQ(dts.getThreadState(),
          static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));

  dts.cancelThread();

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Cancelled));
}

// schedule 2 threads at the same time and tun them
TEST(deferredThreadScheduler, test_5)
{
  using defaultFun = std::function<void()>;
  auto deferredTime = 3s;
  int i{1};

  auto dtsPtr_1 = std::make_shared<deferredThreadScheduler<defaultFun>>("voidFoo");
  dtsPtr_1.get()->registerThread([i]() -> void
                    {
                      utilities::pclog{} << "[ " << __func__ << " ] "
                                << i
                                << ": voidFoo() #1 running: "
                                << "................................."
                                << std::endl;
                    });
  ++i;
  auto dtsPtr_2 = std::make_shared<deferredThreadScheduler<defaultFun>>("voidFoo");
  dtsPtr_2.get()->registerThread([i]() -> void
                    {
                      utilities::pclog{} << "[ " << __func__ << " ] "
                                << i
                                << ": voidFoo() #2 running: "
                                << "................................."
                                << std::endl;
                    });

  dtsPtr_1.get()->runIn(deferredTime);
  dtsPtr_2.get()->runIn(deferredTime);
  dtsPtr_1.get()->wait();
  dtsPtr_2.get()->wait();

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

  ASSERT_EQ(dtsPtr_1.get()->getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
  ASSERT_EQ(dtsPtr_2.get()->getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
}

// schedule many threads at the same time and run them
TEST(deferredThreadScheduler, test_6)
{
  using defaultFun = std::function<void()>;
  using dtsSharedPtr = std::shared_ptr<deferredThreadScheduler<defaultFun>>;
  std::vector<dtsSharedPtr> v {};
  auto deferredTime = 3s;

  for(int i = 1; i <= 10; ++i)
  {
    //auto dtsPtr = makeSharedDeferredThreadScheduler<defaultFun>("voidFoo");
    auto dtsPtr = std::make_shared<deferredThreadScheduler<defaultFun>>("voidFoo");
    dtsPtr.get()->registerThread([i]() -> void
                    {
                      utilities::pclog{} << "[ " << __func__ << " ] "
                                << i
                                << ": voidFoo() running: "
                                << "................................."
                                << std::endl;
                    });
    v.push_back(dtsPtr);
  }
  for (auto& dts : v)
  {
    ASSERT_EQ(dts.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::NotValid));
  }
  for (auto& dts : v)
  {
    ASSERT_EQ(dts.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::NotValid));
    dts.get()->runIn(deferredTime);
  }
  for (auto& dts : v)
  {
    ASSERT_EQ(dts.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));
  }
  for (auto& dts : v)
  {
    dts.get()->wait();
  }
  for (auto& dts : v)
  {
    ASSERT_EQ(dts.get()->getThreadState(),
              static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
  }
}

// schedule many threads at the same time and cancel them

////////////////////////////////////////////////////////////////////////////////
#pragma clang diagnostic pop
// END: ignore the warnings when compiled with clang up to here
