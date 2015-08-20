#include "StdAfx.h"
#include "core/Tools.h"

const char* Tools::getHumanReadableError(long status)
{
  // Put here all the error msgs you need to translate
  // XXX - There *has* to be some correct way to do it directly in the Win32 API, but I don't know how.
  switch (status) {
  case ERROR_SUCCESS: return "ERROR_SUCCESS";
  case PDH_CSTATUS_NO_OBJECT: return "PDH_CSTATUS_NO_OBJECT";
  case PDH_CSTATUS_NO_COUNTER: return "PDH_CSTATUS_NO_COUNTER";
  case PDH_INVALID_HANDLE: return "PDH_INVALID_HANDLE";
  }
  return "ERROR_UNKNOWN";
}
