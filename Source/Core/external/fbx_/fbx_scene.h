#ifndef FBX_SCENE_2_H
#define FBX_SCENE_2_H

#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include <functional>

#include "fbx_node.h"

namespace Fbx {

typedef uint8_t Byte;
typedef uint32_t Word;
typedef uint64_t DoubleWord;
typedef std::function<void(const std::string&)> log_func_t;
typedef std::function<void(const std::string&)> log_warn_func_t;
typedef std::function<void(const std::string&)> log_err_func_t;
typedef std::function<void(const std::string&)> log_dbg_func_t;

void SetLogCallback(log_func_t f);
void SetLogWarnCallback(log_warn_func_t f);
void SetLogErrCallback(log_err_func_t f);
void SetLogDbgCallback(log_dbg_func_t f);

class Scene {
public:
    bool ReadMem(const char* data, size_t size);
    bool ReadFile(const std::string& filename);
private:
    void Finalize(Node& node);

    void Log(const std::string& str);
    void LogWarn(const std::string& str);
    void LogErr(const std::string& str);
    void LogDbg(const std::string& str);
};

}

#endif
