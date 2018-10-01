#ifndef FBX_HELPERS_MACRO_H
#define FBX_HELPERS_MACRO_H

#include <string>
#include <sstream>

#define MKSTR(LINE) \
static_cast<std::ostringstream&>(std::ostringstream() << LINE).str()

#define FBX_LOG(LINE) Log(MKSTR(LINE));
#define FBX_LOGW(LINE) LogWarn("WARNING: " << LINE)
#define FBX_LOGE(LINE) LogErr("ERROR: " << LINE)
#define FBX_LOGD(LINE) LogDbg("DEBUG: " << LINE)

#endif
