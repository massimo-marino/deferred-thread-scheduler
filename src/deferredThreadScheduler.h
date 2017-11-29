/* 
 * File:   deferredThreadScheduler.h
 * Author: massimo
 *
 * Created on November 16, 2017, 10:43 AM
 */
#pragma once

#include <iostream>
#include <type_traits>
#include <string>
#include <tuple>
#include <atomic>
#include <mutex>
#include <future>
#include <thread>
#include <condition_variable>
#include <chrono>
////////////////////////////////////////////////////////////////////////////////
namespace deferredThreadScheduler
{
using namespace std::chrono_literals;

using deafultThreadFunctionResult = int;
using baseThreadStateType = int;

template<typename RT = deafultThreadFunctionResult, typename... Args>
using defaulThreadFun = std::function<RT(const Args&... args)>;

class deferredThreadSchedulerBase
{
public:
  enum class threadState : baseThreadStateType
  {
    NotValid,
    Registered,
    Scheduled,
    Running,
    Run,
    Canceled
  };

  static const std::string version;
  static const std::string& deferredThreadSchedulerVersion() noexcept;

  // we don't want these objects allocated on the heap
  void* operator new(std::size_t) = delete;
  void* operator new[](std::size_t) = delete;
  deferredThreadSchedulerBase() = delete;
  deferredThreadSchedulerBase(const deferredThreadSchedulerBase& rhs) = default;
  deferredThreadSchedulerBase& operator=(const deferredThreadSchedulerBase& rhs) = default;

  explicit
  deferredThreadSchedulerBase(const std::string& threadName) noexcept;

  virtual ~deferredThreadSchedulerBase() noexcept(false);

  const
  std::string&
  getThreadName() const noexcept;

  bool
  cancelThread() const noexcept;

  baseThreadStateType
  getThreadState() const noexcept
  {
    std::lock_guard<std::mutex> lg(threadState_mx_);
    return static_cast<baseThreadStateType>(threadState_);
  }

  constexpr
  bool
  isRegistered(const baseThreadStateType s) noexcept
  {
    return static_cast<baseThreadStateType>(threadState::Registered) == s;
  }
  constexpr
  bool
  isScheduled(const baseThreadStateType s) noexcept
  {
    return static_cast<baseThreadStateType>(threadState::Scheduled) == s;
  }
  constexpr
  bool
  isRunning(const baseThreadStateType s) noexcept
  {
    return static_cast<baseThreadStateType>(threadState::Running) == s;
  }
  constexpr
  bool
  isRun(const baseThreadStateType s) noexcept
  {
    return static_cast<baseThreadStateType>(threadState::Run) == s;
  }
  constexpr
  bool
  isCanceled(const baseThreadStateType s) noexcept
  {
    return static_cast<baseThreadStateType>(threadState::Canceled) == s;
  }

  constexpr
  bool
  isRegistered() const noexcept
  {
    return threadState::Registered == getThreadState_();
  }
  constexpr
  bool
  isScheduled() const noexcept
  {
    return threadState::Scheduled == getThreadState_();
  }
  constexpr
  bool
  isRunning() const noexcept
  {
    return threadState::Running == getThreadState_();
  }
  constexpr
  bool
  isRun() const noexcept
  {
    return threadState::Run == getThreadState_();
  }
  constexpr
  bool
  isCanceled() const noexcept
  {
    return threadState::Canceled == getThreadState_();
  }

  const
  std::thread::id
  getThreadId() const noexcept;

 protected:
  std::string threadName_ {};

  // condition variable for thread cancellation notification
  mutable std::condition_variable cv_ {};
  // mutex associated to the condition variable
  mutable std::mutex cv_mx_ {};

  // mutex associated to the thread state
  mutable std::mutex threadState_mx_ {};
  mutable threadState threadState_ {threadState::NotValid};

  // unused: for padding only
  [[maybe_unused]] const char dummy_ [4] {};

  mutable std::atomic<std::thread::id> threadId_;

  void
  setThreadState(const threadState& threadState) const noexcept;

  threadState
  getThreadState_() const noexcept
  {
    std::lock_guard<std::mutex> lg(threadState_mx_);
    return threadState_;
  }

  void
  setThreadId() const noexcept;
};  // class deferredThreadSchedulerBase

template <typename RT = deafultThreadFunctionResult, typename F = defaulThreadFun<RT>>
class deferredThreadScheduler final : public deferredThreadSchedulerBase
{
 public:
  using threadResult = std::tuple<baseThreadStateType, RT>;

 private:
  mutable std::function<threadResult(const std::chrono::seconds)> f_ {};
  mutable std::shared_future<threadResult> threadFuture_ {};

  const
  std::shared_future<threadResult>&
  getThreadFuture() const noexcept
  {
    return threadFuture_;
  }

 public:
  // we don't want these objects allocated on the heap
  void* operator new(std::size_t) = delete;
  void* operator new[](std::size_t) = delete;
  deferredThreadScheduler() = delete;
  deferredThreadScheduler(const deferredThreadScheduler& rhs) = default;
  deferredThreadScheduler& operator=(const deferredThreadScheduler& rhs) = default;

  ~deferredThreadScheduler()
  {}

  explicit
  deferredThreadScheduler(const std::string& threadName) noexcept
  :
  deferredThreadSchedulerBase(threadName)
  {}

  template <typename... Args>
  explicit
  deferredThreadScheduler(const std::string& threadName, F& f, Args&&... args) noexcept
  :
  deferredThreadSchedulerBase(threadName)
  {
    registerThread(std::forward<F>(f), std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto&
  registerThread(const F& f, Args&&... args) const noexcept
  {
    if ( threadState::NotValid == getThreadState_() )
    {
      // create the closure
      // NOTE:
      //   - f MUST be captured by value; if passed by reference the same lambda
      //     will be defined in all object instances created
      f_ = [this, f, &args...]
           (const std::chrono::seconds& deferredTimeSeconds) noexcept(false) -> threadResult const
           {
             setThreadId();

             std::unique_lock<std::mutex> lk(cv_mx_);
             RT result {};
             // wait on the condition variable until timeout or notification of cancellation
             std::cv_status r = cv_.wait_for(lk, deferredTimeSeconds);
             if ( std::cv_status::timeout == r )
             {
               if ( threadState::Scheduled == getThreadState_() )
               {
                 // run thread function
                 setThreadState(threadState::Running);
                 result = f(std::forward<Args>(args)...);
                 setThreadState(threadState::Run);
               }
             }
             return std::make_tuple(getThreadState(), result);
           };
      setThreadState(threadState::Registered);
    }
    // allow chain calls
    return *this;
  }

  auto&
  runIn(const std::chrono::seconds& deferredTimeSeconds) const noexcept
  {
    if ( threadState::Registered == getThreadState_() )
    {
      setThreadState(threadState::Scheduled);
      // run the closure async
      threadFuture_ = std::async(std::launch::async, f_, deferredTimeSeconds);
    }
    // allow chain calls
    return *this;
  }

  // blocking until the thread terminates or return default values if not in the
  // right state
  threadResult
  wait() const noexcept(false)
  {
    auto ts = getThreadState();

    if ( auto ts_ = getThreadState_();
         ( (threadState::Scheduled == ts_) ||
           (threadState::Running == ts_) ||
           (threadState::Run == ts_)) )
    {
      // an exception thrown inside an async task is propagated when
      // std::future::get() is invoked.
      try
      {
        // wait here the termination
        return getThreadFuture().get();
      }
      catch (...)
      {
        throw;
      }
    }
    // if the thread is not in the right state then return here with default
    // values indicating the thread state and the default return type
    return std::make_tuple(ts, RT{});
  }

  // wait at most ms milliseconds for thread termination, then return no result
  // in case of time-out, otherwise return the result/future if the thread terminated
  threadResult
  wait_for(const std::chrono::milliseconds& ms) const noexcept(false)
  {
    auto ts = getThreadState();
    
    if ( auto ts_ = getThreadState_();
         ( (threadState::Scheduled == ts_) ||
           (threadState::Running == ts_) ||
           (threadState::Run == ts_)) )
    {
      if ( std::future_status::ready == getThreadFuture().wait_for(ms) )
      {
        // an exception thrown inside an async task is propagated when
        // std::future::get() is invoked.
        try
        {
           // wait here the termination
          return getThreadFuture().get();
        }
        catch (...)
        {
          throw;
        }
      }
    }
    // return after time-out: thread not terminated
    return std::make_tuple(ts, RT{});
  }
};  // class deferredThreadScheduler

////////////////////////////////////////////////////////////////////////////////
// factory

template <typename T, typename F>
using deferredThreadSchedulerUniquePtr = std::unique_ptr<deferredThreadScheduler<T, F>>;

// create an object of type T and return a std::unique_ptr to it
template <typename T, typename... Args>
auto
createUniquePtr(Args&&... args) -> std::unique_ptr<T>
{
  return std::make_unique<T>(args...);
}

template <typename T, typename F>
deferredThreadSchedulerUniquePtr<T, F>
makeUniqueDeferredThreadScheduler(const std::string& threadName) noexcept
{
  return createUniquePtr<deferredThreadScheduler<T, F>>(threadName);
}
////////////////////////////////////////////////////////////////////////////////
template <typename T, typename F>
using deferredThreadSchedulerSharedPtr = std::shared_ptr<deferredThreadScheduler<T, F>>;

// create an object of type T and return a std::shared_ptr to it
template <typename T, typename... Args>
auto
createSharedPtr(Args&&... args) -> std::shared_ptr<T>
{
  return std::make_shared<T>(args...);
}

template <typename T, typename F>
deferredThreadSchedulerSharedPtr<T, F>
makeSharedDeferredThreadScheduler(const std::string& threadName) noexcept
{
  return createSharedPtr<deferredThreadScheduler<T, F>>(threadName);
}
}  // namespace deferredThreadScheduler
