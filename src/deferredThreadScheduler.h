/* 
 * File:   deferredThreadScheduler.h
 * Author: massimo
 *
 * Created on November 16, 2017, 10:43 AM
 */
#ifndef DEFERRED_THREAD_SCHEDULER_H
#define DEFERRED_THREAD_SCHEDULER_H
#include <iostream>
#include <string>
#include <mutex>
#include <future>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <unistd.h>
////////////////////////////////////////////////////////////////////////////////
namespace deferredThreadScheduler
{
using namespace std::chrono_literals;

class deferredThreadSchedulerBase
{
public:
  enum class threadState : int { NotValid, Scheduled, Run, Running, Cancelled };
  static const std::string version;
  static const std::string& deferredThreadSchedulerVersion() noexcept;

  // we don't want these objects allocated on the heap
  void* operator new(std::size_t) = delete;
  void* operator new[](std::size_t) = delete;
  deferredThreadSchedulerBase() = delete;
  deferredThreadSchedulerBase(const deferredThreadSchedulerBase& rhs) = default;
  deferredThreadSchedulerBase& operator=(const deferredThreadSchedulerBase& rhs) = default;

  explicit
  deferredThreadSchedulerBase(const std::string& threadName) noexcept
  :
  threadName_ (threadName)
  {}

  virtual ~deferredThreadSchedulerBase();

  const
  std::string&
  getThreadName() const noexcept
  {
    return threadName_;
  }

  void
  cancelThread() const noexcept
  {
    {
      std::unique_lock<std::mutex> lk(cv_m_);
      if ( (threadState::NotValid != getThreadState_()) &&
           (threadState::Scheduled != getThreadState_()) )
      {
        return;
      }
      setThreadState(threadState::Cancelled);
    }
    cv_.notify_one();
  }

  constexpr
  auto
  wait() const noexcept
  {
    return getThreadFuture().get();
  }

  constexpr
  int
  getThreadState() const noexcept
  {
    return static_cast<int>(threadState_);
  }

  const
  std::thread::id
  getThreadId() const noexcept
  {
    return threadId_;
  }

 protected:
  std::string threadName_ {};

  // condition variable for thread cancellation notification
  mutable std::condition_variable cv_ {};
  // mutex associated to the condition variable
  mutable std::mutex cv_m_ {};

  mutable threadState threadState_ {threadState::NotValid};
  mutable std::thread::id threadId_ {};
  mutable std::shared_future<int> threadFuture_ {};

  void
  setThreadState(const threadState& threadState) const noexcept
  {
    threadState_ = threadState; 
  }

  constexpr
  threadState
  getThreadState_() const noexcept
  {
    return threadState_;
  }

  const
  std::shared_future<int>&
  getThreadFuture() const noexcept
  {
    return threadFuture_;
  }

  void
  setThreadId() const noexcept
  {
    threadId_ = std::this_thread::get_id();
  }
};  // class deferredThreadSchedulerBase

using defaultFun = std::function<void()>;

template <typename F = defaultFun>
class deferredThreadScheduler final : public deferredThreadSchedulerBase
{
 private:
  mutable std::function<int(const std::chrono::seconds)> f_ {};

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

  void
  registerThread(const F& f) const noexcept
  {
    // create the closure
    // NOTES: 
    //   - deferredTimeSeconds MUST be captured by value, NOT by reference
    //     Reason: testing has shown that if captured by reference the call to wait_for()
    //     behaves weirdly (the timer seems messed up)
    //   - f MUST be captured by value; if passed by reference the same lambda
    //     will be defined in all object instances created
    f_ = [this, f](const std::chrono::seconds deferredTimeSeconds) noexcept(false) -> int
    {
      setThreadId();

      std::unique_lock<std::mutex> lk(cv_m_);
      // wait on the condition variable until timeout or notification of cancellation
      std::cv_status r = cv_.wait_for(lk, deferredTimeSeconds);
      if ( std::cv_status::timeout == r )
      {
        if ( threadState::Scheduled == getThreadState_() )
        {
          // run thread function
          setThreadState(threadState::Running),
          f(),
          setThreadState(threadState::Run);
        }
      }
      return getThreadState();
    };    
  }

  auto
  runIn(const std::chrono::seconds deferredTimeSeconds) const noexcept
  {
    // run the closure async
    threadFuture_ = std::async(std::launch::async, f_, deferredTimeSeconds);
    setThreadState(threadState::Scheduled);

    // allow chain calls
    return this;
  }
};  // class deferredThreadScheduler

////////////////////////////////////////////////////////////////////////////////
// factory

template <typename F>
using deferredThreadSchedulerUniquePtr = std::unique_ptr<deferredThreadScheduler<F>>;

// create an object of type T and return a std::unique_ptr to it
template <typename T, typename... Args>
auto
createUniquePtr(Args&&... args) -> std::unique_ptr<T>
{
  return std::make_unique<T>(args...);
}

template <typename F>
deferredThreadSchedulerUniquePtr<F>
makeUniqueDeferredThreadScheduler(const std::string& threadName) noexcept
{
  return createUniquePtr<deferredThreadScheduler<F>>(threadName);
}
////////////////////////////////////////////////////////////////////////////////
template <typename F>
using deferredThreadSchedulerSharedPtr = std::shared_ptr<deferredThreadScheduler<F>>;

// create an object of type T and return a std::shared_ptr to it
template <typename T, typename... Args>
auto
createSharedPtr(Args&&... args) -> std::shared_ptr<T>
{
  return std::make_shared<T>(args...);
}

template <typename F>
deferredThreadSchedulerSharedPtr<F>
makeSharedDeferredThreadScheduler(const std::string& threadName) noexcept
{
  return createSharedPtr<deferredThreadScheduler<F>>(threadName);
}
}  // namespace deferredThreadScheduler
#endif /* DEFERRED_THREAD_SCHEDULER_H */
