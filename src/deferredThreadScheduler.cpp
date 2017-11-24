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
{}

deferredThreadSchedulerBase::~deferredThreadSchedulerBase()
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
    std::unique_lock<std::mutex> lk(cv_m_);
    if ( (threadState::NotValid != getThreadState_()) &&
         (threadState::Registered != getThreadState_()) &&
         (threadState::Scheduled != getThreadState_()) )
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
  threadId_ = std::this_thread::get_id();
}

const
std::thread::id
deferredThreadSchedulerBase::getThreadId() const noexcept
{
  return threadId_;
}

void
deferredThreadSchedulerBase::setThreadState(const threadState& threadState) const noexcept
{
  threadState_ = threadState; 
}

}  // namespace deferredThreadScheduler
////////////////////////////////////////////////////////////////////////////////
#pragma clang diagnostic pop
// END: ignore the warnings when compiled with clang up to here
