/* 
 * File:   deferredThreadScheduler.h
 * Author: massimo
 *
 * Created on November 16, 2017, 10:43 AM
 */
#ifndef DEFERRED_THREAD_SCHEDULER_H
#define DEFERRED_THREAD_SCHEDULER_H
#include <iostream>
#include <type_traits>
#include <string>
#include <tuple>
#include <mutex>
#include <future>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <unistd.h>
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

  virtual ~deferredThreadSchedulerBase();

  const
  std::string&
  getThreadName() const noexcept;

  bool
  cancelThread() const noexcept;

  constexpr
  baseThreadStateType
  getThreadState() const noexcept
  {
    return static_cast<baseThreadStateType>(threadState_);
  }

  const
  std::thread::id
  getThreadId() const noexcept;

 protected:
  std::string threadName_ {};

  // condition variable for thread cancellation notification
  mutable std::condition_variable cv_ {};
  // mutex associated to the condition variable
  mutable std::mutex cv_m_ {};

  mutable threadState threadState_ {threadState::NotValid};
  // unused: for padding only
  [[maybe_unused]] const char dummy_ [4] {};
  mutable std::thread::id threadId_ {};

  void
  setThreadState(const threadState& threadState) const noexcept;

  constexpr
  threadState
  getThreadState_() const noexcept
  {
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
             RT result {};

             setThreadId();

             std::unique_lock<std::mutex> lk(cv_m_);
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
      // run the closure async
      threadFuture_ = std::async(std::launch::async, f_, deferredTimeSeconds);
      setThreadState(threadState::Scheduled);
    }
    // allow chain calls
    return *this;
  }

  // blocking until the thread terminates
  constexpr
  threadResult
  wait() const noexcept
  {
    if ( (threadState::Scheduled == getThreadState_()) ||
         (threadState::Running == getThreadState_()) ||
         (threadState::Run == getThreadState_()) )
    {
      return getThreadFuture().get();
    }
    return std::make_tuple(getThreadState(), RT{});
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
#endif /* DEFERRED_THREAD_SCHEDULER_H */
