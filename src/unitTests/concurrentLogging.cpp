/* 
 * File:   concurrentLogging.cpp
 * Author: massimo
 *
 * Created on October 13, 2017, 08:30 AM
 */

#include "concurrentLogging.h"
////////////////////////////////////////////////////////////////////////////////
namespace utilities
{
pclog::~pclog()
{
  std::lock_guard<std::mutex> lg {clog_mutex};
  std::clog << rdbuf();
  std::clog.flush();
}

pcout::~pcout()
{
  std::lock_guard<std::mutex> lg {cout_mutex};
  std::cout << rdbuf();
  std::cout.flush();
}

pcerr::~pcerr()
{
  std::lock_guard<std::mutex> lg {cerr_mutex};
  std::cerr << rdbuf();
  std::cerr.flush();
}

}  // namespace utilities
