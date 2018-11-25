#ifndef FBX_HELPERS_MACRO_H
#define FBX_HELPERS_MACRO_H

#include <string>
#include <sstream>

#define MKSTR(LINE) \
static_cast<std::ostringstream&>(std::ostringstream() << LINE).str()

#define FBX_LOG(LINE) Log(MKSTR(LINE));
#define FBX_LOGW(LINE) LogWarn(MKSTR("WARNING: " << LINE))
#define FBX_LOGE(LINE) LogErr(MKSTR("ERROR: " << LINE))
#define FBX_LOGD(LINE) LogDbg(MKSTR("DEBUG: " << LINE))

#endif
