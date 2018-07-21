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
#include <map>
#include <atomic>
#include <mutex>
#include <future>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <ratio>
////////////////////////////////////////////////////////////////////////////////
// BEGIN: ignore the warnings listed below when compiled with clang from here
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wpadded"
////////////////////////////////////////////////////////////////////////////////
namespace deferredThreadScheduler
{
using namespace std::chrono_literals;

// reallyAsync() from Scott Meyers' Effective Modern C++, pag.250
template<typename F, typename... Ts>
inline
auto // C++14
reallyAsync(F&& f, Ts&&... params)
{
  return std::async(std::launch::async,
                    std::forward<F>(f),
                    std::forward<Ts>(params)...);
}

using deafultThreadFunctionResult = int;
using baseThreadStateType = int;

template<typename RT = deafultThreadFunctionResult, typename... Args>
using defaulThreadFun = std::function<RT(const Args&... args)>;

using uniqueKey = std::thread::id;
using cflag = bool;
using cflags = std::map<uniqueKey, cflag>;

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
    Canceled,
    ExceptionThrown
  };

  static inline std::string version {"1.0.0"};
  static std::string& deferredThreadSchedulerVersion() noexcept;

  deferredThreadSchedulerBase() = delete;
  deferredThreadSchedulerBase(const deferredThreadSchedulerBase& rhs) = delete;
  deferredThreadSchedulerBase& operator=(const deferredThreadSchedulerBase& rhs) = delete;
  deferredThreadSchedulerBase(deferredThreadSchedulerBase&& rhs) = delete;
  deferredThreadSchedulerBase& operator=(deferredThreadSchedulerBase&& rhs) = delete;

  explicit
  deferredThreadSchedulerBase(const std::string& threadName) noexcept;

  virtual ~deferredThreadSchedulerBase() noexcept(false);

  std::string&
  getThreadName() const noexcept;

  bool
  cancelThread() const noexcept;

  baseThreadStateType
  getThreadState() const noexcept;

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
  isExceptionThrown(const baseThreadStateType s) noexcept
  {
    return static_cast<baseThreadStateType>(threadState::ExceptionThrown) == s;
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

  constexpr
  bool
  isExceptionThrown() const noexcept
  {
    return threadState::ExceptionThrown == getThreadState_();
  }

  std::thread::id
  getThreadId() const noexcept;

  std::string
  getExceptionThrownMessage() const noexcept;

  static
  bool
  isCancellationFlagSet(const uniqueKey& uk) noexcept
  {
    return true == getCancellationFlag(uk);
  }
  static
  bool
  isCancellationFlagSet() noexcept
  {
    return true == getCancellationFlag(std::this_thread::get_id());
  }

  static
  auto
  listCancellationFlags(std::ostream& os) noexcept
  {
    unsigned int cancellationFlagSet {};
    unsigned int cancellationFlagUnSet {};

    std::unique_lock<std::mutex> lk(cancellationFlagsMx_);
    if ( 0 == getCancellationFlags_ref().size() )
    {
      os << "[" << __func__ << "] "
         << "Cancellation flags map is EMPTY"
         << std::endl;
    }
    for (auto&& [uniqueKey, cancellationFlag] : getCancellationFlags_ref())
    {
      os << "[" << __func__ << "] "
         << uniqueKey
         << ": "
         << std::boolalpha
         << cancellationFlag
         << std::endl;
      if ( cancellationFlag )
      {
        ++cancellationFlagSet;
      }
      else
      {
        ++cancellationFlagUnSet;
      }
    }
    return std::make_tuple(getCancellationFlags_ref().size(),
                           cancellationFlagSet,
                           cancellationFlagUnSet);
  }

 protected:
  // mutex associated to cancellation flags static map
  static inline std::mutex cancellationFlagsMx_ {};

  // the cancellation flags static map
  // it's static because it is a class attribute
  static cflags cancellationFlags_;

  mutable std::string threadName_ {};

  // condition variable for thread cancellation notification
  mutable std::condition_variable cv_ {};
  // mutex associated to the condition variable cv_
  mutable std::mutex cv_mx_ {};

  // mutex associated to the thread state
  mutable std::mutex threadState_mx_ {};
  mutable threadState threadState_ {threadState::NotValid};
  mutable std::atomic<std::thread::id> threadId_;

  mutable std::string exceptionThrownMessage_ {};

  void
  setExceptionThrownMessage(const std::string& s) const noexcept;

  void
  setThreadState(const threadState& threadState) const noexcept;

  threadState
  getThreadState_() const noexcept;

  void
  setThreadId() const noexcept;

  static
  cflags&
  getCancellationFlags_ref() noexcept
  {
    return cancellationFlags_;
  }

  static
  void
  setCancellationFlag(const uniqueKey& uk) noexcept
  {
    std::unique_lock<std::mutex> lk(cancellationFlagsMx_);
    getCancellationFlags_ref()[uk] = true;
  }
  static
  void
  setCancellationFlag() noexcept
  {
    std::unique_lock<std::mutex> lk(cancellationFlagsMx_);
    getCancellationFlags_ref()[std::this_thread::get_id()] = true;
  }

  static
  cflag
  getCancellationFlag(const uniqueKey& uk) noexcept
  {
    std::unique_lock<std::mutex> lk(cancellationFlagsMx_);
    return getCancellationFlags_ref()[uk];
  }
  static
  cflag
  getCancellationFlag() noexcept
  {
    std::unique_lock<std::mutex> lk(cancellationFlagsMx_);
    return getCancellationFlags_ref()[std::this_thread::get_id()];
  }

  static
  auto
  eraseCancellationFlag(const uniqueKey& uk) noexcept
  {
    // remove the entry for this thread from the static map
    std::unique_lock<std::mutex> lk(cancellationFlagsMx_);
    return getCancellationFlags_ref().erase(uk);
  }
};  // class deferredThreadSchedulerBase

#define TERMINATE_ON_CANCELLATION(THREAD_RETURN_TYPE) \
if ( deferredThreadSchedulerBase::isCancellationFlagSet() ) \
{ \
  return THREAD_RETURN_TYPE {}; \
} \

template <typename RT = deafultThreadFunctionResult, typename F = defaulThreadFun<RT>>
class deferredThreadScheduler final : public deferredThreadSchedulerBase
{
  // to make things simpler the thread function must return a value
  static_assert(!std::is_void<RT>::value,
                "The return value of the thread function must not be void");

 public:
  using threadResult = std::tuple<baseThreadStateType, RT>;
  using deferredTimeGranularity = std::chrono::nanoseconds;

private:
  mutable std::function<threadResult(const deferredTimeGranularity)> f_ {};
  mutable std::shared_future<threadResult> threadFuture_ {};

  std::shared_future<threadResult>&
  getThreadFuture() const noexcept
  {
    return threadFuture_;
  }

  void
  setThreadFuture(const std::shared_future<threadResult>& r) const noexcept
  {
    threadFuture_ = r;
  }

 public:
  deferredThreadScheduler() = delete;
  deferredThreadScheduler(const deferredThreadScheduler& rhs) = delete;
  deferredThreadScheduler& operator=(const deferredThreadScheduler& rhs) = delete;
  deferredThreadScheduler(deferredThreadScheduler&& rhs) = delete;
  deferredThreadScheduler& operator=(deferredThreadScheduler&& rhs) = delete;

  ~deferredThreadScheduler() noexcept
  {
    auto tid = getThreadId();
    // the dtor MUST NEVER be called manually.
    // Remember: the std::future returned from std::async will block in its
    // destructor until the asynchronously running thread has completed
    // So, here, if the thread is still running we must force its termination by
    // setting the cancellation flag and waiting its termination.
    // This works only if the thread calls the static method isCancellationFlagSet()
    // at a safe cancellation point of its code; otherwise the thread continues
    // executing and the dtor never ends
    if ( isRunning() )
    {
      // this to notify the thread that must call isCancellationFlagSet() at a
      // safe cancellation point of its code to verify its cancellation flag was set
      setCancellationFlag(tid);
      // then the thread must terminate and the dtor blocks here until done
      terminate(tid);   
      return;
    }
    // remove the entry for this thread from the static map
    eraseCancellationFlag(tid);
  }

  auto
  terminate(const uniqueKey& tid) const noexcept(false)
  {
    auto r = getThreadFuture().get();

    // remove the entry for this thread from the static map
    eraseCancellationFlag(tid);
    return r;
  }
  constexpr
  auto
  terminate() const noexcept(false)
  {
    return terminate(getThreadId());
  }

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
           (const deferredTimeGranularity deferredTime) noexcept(false) -> threadResult const
           {
             RT result {};

             setThreadId();
             std::unique_lock<std::mutex> lk(cv_mx_);
             // wait on the condition variable until timeout or notification of cancellation
             if ( std::cv_status r = cv_.wait_for(lk, deferredTime);
                  std::cv_status::timeout == r )
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
  runIn(const double deferredTimeSeconds) const noexcept
  {
    return runIn(std::chrono::duration_cast<deferredTimeGranularity>(std::chrono::duration<double>{deferredTimeSeconds}));
  }
  auto&
  runIn(const std::chrono::seconds deferredTime) const noexcept
  {
    return runIn(std::chrono::duration_cast<deferredTimeGranularity>(deferredTime));
  }
  auto&
  runIn(const std::chrono::milliseconds deferredTime) const noexcept
  {
    return runIn(std::chrono::duration_cast<deferredTimeGranularity>(deferredTime));
  }
  auto&
  runIn(const std::chrono::microseconds deferredTime) const noexcept
  {
    return runIn(std::chrono::duration_cast<deferredTimeGranularity>(deferredTime));
  }
  auto&
  runIn(const deferredTimeGranularity deferredTime) const noexcept
  {
    if ( threadState::Registered == getThreadState_() )
    {
      setThreadState(threadState::Scheduled);
      // run the closure async
      setThreadFuture(reallyAsync(f_, deferredTime));
    }
    // allow chain calls
    return *this;
  }

  // blocking until the thread terminates or return default values if not in the
  // right state
  threadResult
  wait() const noexcept(false)
  {
    auto ts {getThreadState()};

    if ( auto ts_ {getThreadState_()};
         ( (threadState::Scheduled == ts_) ||
           (threadState::Running == ts_) ||
           (threadState::Run == ts_)) )
    {
      // an exception thrown inside an async task is propagated when
      // std::future::get() is invoked.
      try
      {
        // wait here the termination
        return terminate();
      }
      catch (const std::exception& e)
      {
        // remove the entry for this thread from the static map
        eraseCancellationFlag(getThreadId());
        setExceptionThrownMessage(e.what());
        setThreadState(threadState::ExceptionThrown);
        return std::make_tuple(getThreadState(), RT {});
      }
    }
    // if the thread is not in the right state then return here with default
    // values indicating the thread state and the default return type
    return std::make_tuple(ts, RT{});
  }

  // wait at most s seconds for thread termination, then return no result
  // in case of time-out, otherwise return the result/future if the thread terminated
  threadResult
  wait_for(const std::chrono::seconds s) const noexcept(false)
  {
    return wait_for(std::chrono::duration_cast<std::chrono::nanoseconds>(s));
  }
  // wait at most ms milliseconds for thread termination, then return no result
  // in case of time-out, otherwise return the result/future if the thread terminated
  threadResult
  wait_for(const std::chrono::milliseconds ms) const noexcept(false)
  {
    return wait_for(std::chrono::duration_cast<std::chrono::nanoseconds>(ms));
  }
  // wait at most us microseconds for thread termination, then return no result
  // in case of time-out, otherwise return the result/future if the thread terminated
  threadResult
  wait_for(const std::chrono::microseconds us) const noexcept(false)
  {
    return wait_for(std::chrono::duration_cast<std::chrono::nanoseconds>(us));
  }
  // wait at most ns nanoseconds for thread termination, then return no result
  // in case of time-out, otherwise return the result/future if the thread terminated
  threadResult
  wait_for(const std::chrono::nanoseconds ns = 0ns) const noexcept(false)
  {
    auto ts {getThreadState()};

    if ( auto ts_ {getThreadState_()};
         ( (threadState::Scheduled == ts_) ||
           (threadState::Running == ts_) ||
           (threadState::Run == ts_)) )
    {
      if ( std::future_status::ready == getThreadFuture().wait_for(ns) )
      {
        // an exception thrown inside an async task is propagated when
        // std::future::get() is invoked.
        try
        {
          // wait here the termination
          return terminate();
        }
        catch (const std::exception& e)
        {
          // remove the entry for this thread from the static map
          eraseCancellationFlag(getThreadId());
          setExceptionThrownMessage(e.what());
          setThreadState(threadState::ExceptionThrown);
          return std::make_tuple(getThreadState(), RT {});
        }
      }
    }
    // return after time-out: thread not terminated
    return std::make_tuple(ts, RT{});
  }

  static
  bool
  isCancellationFlagSet(const uniqueKey& uk) noexcept
  {
    return true == getCancellationFlag(uk);
  }
  static
  bool
  isCancellationFlagSet() noexcept
  {
    return true == getCancellationFlag(std::this_thread::get_id());
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
