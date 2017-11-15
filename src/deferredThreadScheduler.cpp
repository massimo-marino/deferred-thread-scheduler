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
////////////////////////////////////////////////////////////////////////////////
//
// deferredThreadSchedulerVersion
//
// Purpose: Return the library version
//
const std::string& deferredThreadSchedulerBase::deferredThreadSchedulerVersion () noexcept
{
  return version;
}

deferredThreadSchedulerBase::~deferredThreadSchedulerBase()
{}

}  // namespace deferredThreadScheduler
////////////////////////////////////////////////////////////////////////////////
#pragma clang diagnostic pop
// END: ignore the warnings when compiled with clang up to here
