#ifndef FBX_LOG_H
#define FBX_LOG_H

#include <functional>
#include <string>

namespace Fbx {

typedef std::function<void(const std::string&)> log_func_t;
typedef std::function<void(const std::string&)> log_warn_func_t;
typedef std::function<void(const std::string&)> log_err_func_t;
typedef std::function<void(const std::string&)> log_dbg_func_t;

void Log(const std::string& str);
void LogWarn(const std::string& str);
void LogErr(const std::string& str);
void LogDbg(const std::string& str);

void SetLogCallback(log_func_t f);
void SetLogWarnCallback(log_warn_func_t f);
void SetLogErrCallback(log_err_func_t f);
void SetLogDbgCallback(log_dbg_func_t f);

}

#endif
