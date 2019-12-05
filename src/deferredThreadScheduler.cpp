/* 
 * File:   deferredThreadScheduler.cpp
 * Author: massimo
 *
 * Created on November 16, 2017, 10:43 AM
 */
#include "deferredThreadScheduler.h"
////////////////////////////////////////////////////////////////////////////////
namespace DTS
{
cflags deferredThreadSchedulerBase::cancellationFlags_ {};

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

std::thread::id
deferredThreadSchedulerBase::getThreadId() const noexcept
{
  return threadId_.load();
}

std::string
deferredThreadSchedulerBase::getExceptionThrownMessage() const noexcept
{
  return exceptionThrownMessage_;
}

void
deferredThreadSchedulerBase::setExceptionThrownMessage(const std::string& s) const noexcept
{
  exceptionThrownMessage_ = s;
}

void
deferredThreadSchedulerBase::setThreadState(const threadState& threadState) const noexcept
{
  std::lock_guard<std::mutex> lg(threadState_mx_);
  threadState_ = threadState; 
}

baseThreadStateType
deferredThreadSchedulerBase::getThreadState() const noexcept
{
  std::lock_guard<std::mutex> lg(threadState_mx_);
  return static_cast<baseThreadStateType>(threadState_);
}

deferredThreadSchedulerBase::threadState
deferredThreadSchedulerBase::getThreadState_() const noexcept
{
  std::lock_guard<std::mutex> lg(threadState_mx_);
  return threadState_;
}
}  // namespace DTS
////////////////////////////////////////////////////////////////////////////////
#pragma clang diagnostic pop
// END: ignore the warnings when compiled with clang up to here
