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

  deferredThreadSchedulerBase(const std::string& threadName)
  :
  threadName_ (threadName)
  {}

  virtual ~deferredThreadSchedulerBase();

  void
  cancelThread() const noexcept
  {
    {
      std::unique_lock<std::mutex> lk(cv_m_);
      if ( (threadState::NotValid == getThreadState_()) ||
           (threadState::Scheduled == getThreadState_()) )
      {
        setThreadState(threadState::Cancelled);
      }
      else
      {
        return;
      }
    }
    cv_.notify_one();
  }

  const
  std::string&
  getThreadName() const noexcept
  {
    return threadName_;
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

 protected:
  mutable threadState threadState_ {threadState::NotValid};
  // condition variable for thread cancellation notification
  mutable std::condition_variable cv_;
  // mutex associated to the condition variable
  mutable std::mutex cv_m_;
  const std::string threadName_ {};
  mutable std::future<int> threadFuture_ {};
  std::function<int()> f_ {};

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

  constexpr
  std::future<int>&
  getThreadFuture() const noexcept
  {
    return threadFuture_;
  }
};  // class deferredThreadSchedulerBase

using defaultFun = std::function<void()>;

template <typename F = defaultFun>
class deferredThreadScheduler final : public deferredThreadSchedulerBase
{
 public:
  // we don't want these objects allocated on the heap
  void* operator new(std::size_t) = delete;
  deferredThreadScheduler() = delete;
  ~deferredThreadScheduler()
  {}

  explicit
  deferredThreadScheduler(const std::string& threadName,
                          const F& f,
                          const std::chrono::seconds& deferredTimeSeconds) noexcept
  :
  deferredThreadSchedulerBase(threadName)
  {
    // create the lambda
    // NOTE: deferredTimeSeconds MUST be captured by value, NOT by reference
    f_ = [this, &f, deferredTimeSeconds]() noexcept -> int
    {
      std::unique_lock<std::mutex> lk(cv_m_);
      // wait on the condition variable until timeout or notification of cancellation
      std::cv_status r = cv_.wait_for(lk, deferredTimeSeconds);
      if ( std::cv_status::timeout == r )
      {
        // run thread function
        setThreadState(threadState::Running),
        f(),
        setThreadState(threadState::Run);
      }
      return getThreadState();
    };
    // run the lambda async
    threadFuture_ = std::async(std::launch::async, f_);
    setThreadState(threadState::Scheduled);
  }
};  // class deferredThreadScheduler

////////////////////////////////////////////////////////////////////////////////
// factory

template <typename F>
using deferredThreadSchedulerPtr = std::unique_ptr<deferredThreadScheduler<F>>;

// create an object of type T and return a std::unique_ptr to it
template <typename T, typename... Args>
auto
createUniquePtr(Args&&... args) -> std::unique_ptr<T>
{
  return std::make_unique<T>(args...);
}

template <typename F>
deferredThreadSchedulerPtr<F>
makeDeferredThreadScheduler(const std::string& threadName,
                            const F& f,
                            const std::chrono::seconds& deferredTimeSeconds) noexcept
{
  return createUniquePtr<deferredThreadScheduler<F>>(threadName, f, deferredTimeSeconds);
}
}  // namespace deferredThreadScheduler
#endif /* DEFERRED_THREAD_SCHEDULER_H */
