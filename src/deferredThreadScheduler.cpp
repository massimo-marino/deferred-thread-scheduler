/* 
 * File:   deferredThreadScheduler.cpp
 * Author: massimo
 *
 * Created on November 16, 2017, 10:43 AM
 */
#include "deferredThreadScheduler.h"
////////////////////////////////////////////////////////////////////////////////
// BEGIN: ignore the warnings listed below when compiled with clang from here
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
////////////////////////////////////////////////////////////////////////////////
namespace deferredThreadScheduler
{
std::string const deferredThreadSchedulerBase::version = "1.0.0";

const
std::string&
deferredThreadSchedulerBase::deferredThreadSchedulerVersion () noexcept
{
  return version;
}

deferredThreadSchedulerBase::deferredThreadSchedulerBase(const std::string& threadName) noexcept
:
threadName_ (threadName)
{
  std::atomic_init(&threadId_, {});
}

deferredThreadSchedulerBase::~deferredThreadSchedulerBase() noexcept(false)
{}

const
std::string&
deferredThreadSchedulerBase::getThreadName() const noexcept
{
  return threadName_;
}

bool
deferredThreadSchedulerBase::cancelThread() const noexcept
{
  {
    std::unique_lock<std::mutex> lk(cv_mx_);
    if ( auto ts_ = getThreadState_();
         (threadState::NotValid != ts_) &&
         (threadState::Registered != ts_) &&
         (threadState::Scheduled != ts_) )
    {
      // thread cannot be canceled when not possible
      return false;
    }
    setThreadState(threadState::Canceled);
  }
  cv_.notify_one();
  return true;
}

void
deferredThreadSchedulerBase::setThreadId() const noexcept
{
  threadId_.store(std::this_thread::get_id());
}

const
std::thread::id
deferredThreadSchedulerBase::getThreadId() const noexcept
{
  return threadId_.load();
}

void
deferredThreadSchedulerBase::setThreadState(const threadState& threadState) const noexcept
{
  std::lock_guard<std::mutex> lg(threadState_mx_);
  threadState_ = threadState; 
}

}  // namespace deferredThreadScheduler
////////////////////////////////////////////////////////////////////////////////
#pragma clang diagnostic pop
// END: ignore the warnings when compiled with clang up to here
