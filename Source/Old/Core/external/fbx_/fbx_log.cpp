#include "fbx_log.h"

namespace Fbx {

static log_func_t log_func;
static log_warn_func_t log_warn_func;
static log_err_func_t log_err_func;
static log_dbg_func_t log_dbg_func;

void Log(const std::string& str) {
    if(log_func) log_func(str);
}
void LogWarn(const std::string& str) {
    if(log_warn_func) log_warn_func(str);
}
void LogErr(const std::string& str) {
    if(log_err_func) log_err_func(str);
}
void LogDbg(const std::string& str) {
    if(log_dbg_func) log_dbg_func(str);
}

void SetLogCallback(log_func_t f) {
    log_func = f;
}
void SetLogWarnCallback(log_warn_func_t f) {
    log_warn_func = f;
}
void SetLogErrCallback(log_err_func_t f) {
    log_err_func = f;
}
void SetLogDbgCallback(log_err_func_t f) {
    log_err_func = f;
}

} // Fbx
