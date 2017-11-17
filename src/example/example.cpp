/* 
 * File:   example.cpp
 * Author: massimo
 * 
 * Created on November 17, 2017, 11:57 AM
 */
#include "deferredThreadScheduler.h"
#include <iostream>
////////////////////////////////////////////////////////////////////////////////
auto main() -> int
{
  std::clog << "\n[" << __func__ << "] "
            << "Deferred Thread Scheduler Example STARTED"
            << std::endl;

  std::clog << "\n[" << __func__ << "] "
            << "TERMINATED"
            << std::endl;
}  // main
