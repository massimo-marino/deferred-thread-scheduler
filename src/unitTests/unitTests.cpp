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
    std::cout << "[ " << __func__ << " ] "
              << "voidFoo() running................................."
              << std::endl;
  };

  deferredThreadScheduler<defaultFun> dts {"voidFoo", voidFoo, 5s};

  ASSERT_EQ(dts.getThreadState(),
          static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));

  std::cout << "[ " << __func__ << " ] "
             << "Thread name: "
            << dts.getThreadName() << std::endl;
  std::cout << "[ " << __func__ << " ] "
            << "waiting for thread "
            << dts.getThreadName()
            << " to run..." << std::endl;
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
    std::cout << "[ " << __func__ << " ] "
              << "voidFoo() running: "
              << threadArg
              << "................................."
              << std::endl;
  };

  deferredThreadScheduler<defaultFun> dts {"voidFoo", voidFoo, 2s};

  ASSERT_EQ(dts.getThreadState(),
        static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));

  std::cout << "[ " << __func__ << " ] "
             << "Thread name: "
            << dts.getThreadName() << std::endl;
  std::cout << "[ " << __func__ << " ] "
            << "waiting for thread "
            << dts.getThreadName()
            << " to run..." << std::endl;
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
    std::cout << "[ " << __func__ << " ] "
              << "voidFoo() running: "
              << threadArg
              << "................................."
              << std::endl;
  };

  deferredThreadScheduler<defaultFun> dts {"voidFoo", voidFoo, 2s};

  ASSERT_EQ(dts.getThreadState(),
        static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));

  std::cout << "[ " << __func__ << " ] "
             << "Thread name: "
            << dts.getThreadName() << std::endl;
  std::cout << "[ " << __func__ << " ] "
            << "waiting for thread "
            << dts.getThreadName()
            << " to run..." << std::endl;
  dts.wait();

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
  ASSERT_EQ(threadArg, "Hello World... ");
}

void voidFooGlobal();
void voidFooGlobal()
{
  std::cout << "[ " << __func__ << " ] "
            << "voidFooGlobal() running................................."
            << std::endl;
};

TEST(deferredThreadScheduler, test_3)
{
  deferredThreadScheduler<void()> dts {"voidFooGlobal", voidFooGlobal, 3s};

  ASSERT_EQ(dts.getThreadState(),
          static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));

  std::cout << "[ " << __func__ << " ] "
             << "Thread name: "
            << dts.getThreadName() << std::endl;
  std::cout << "[ " << __func__ << " ] "
            << "waiting for thread "
            << dts.getThreadName()
            << " to run..." << std::endl;
  dts.wait();

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Run));
}

TEST(deferredThreadScheduler, test_4)
{
  deferredThreadScheduler<void()> dts {"voidFooGlobal", voidFooGlobal, 60s};

  ASSERT_EQ(dts.getThreadState(),
          static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));

  std::cout << "[ " << __func__ << " ] "
            << "Thread name: "
            << dts.getThreadName() << std::endl;
  std::cout << "[ " << __func__ << " ] "
            << "waiting for thread "
            << dts.getThreadName()
            << " to run..." << std::endl;

  // cancel the thread after 7 seconds; the test should take at least 7 seconds
  // and a bit more, but not 60 seconds as requested at the declaration of dts
  std::this_thread::sleep_for(7s);
  ASSERT_EQ(dts.getThreadState(),
          static_cast<int>(deferredThreadSchedulerBase::threadState::Scheduled));

  dts.cancelThread();

  ASSERT_EQ(dts.getThreadState(),
            static_cast<int>(deferredThreadSchedulerBase::threadState::Cancelled));
}
////////////////////////////////////////////////////////////////////////////////
#pragma clang diagnostic pop
// END: ignore the warnings when compiled with clang up to here
