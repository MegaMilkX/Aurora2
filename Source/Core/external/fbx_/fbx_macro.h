#ifndef FBX_HELPERS_MACRO_H
#define FBX_HELPERS_MACRO_H

#include <string>
#include <sstream>

#define MKSTR(LINE) \
static_cast<std::ostringstream&>(std::ostringstream() << LINE).str()

#define LOG(LINE) Log(MKSTR(LINE << std::endl));
#define LOG_WARN(LINE) LogWarn("WARNING: " << LINE << std::endl)
#define LOG_ERR(LINE) LogErr("ERROR: " << LINE << std::endl)
#define LOG_DBG(LINE) LogDbg("DEBUG: " << LINE << std::endl)

#endif
