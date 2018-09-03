#ifndef AURORA2_LOG_H
#define AURORA2_LOG_H

#include <string>
#include <iostream>

class LogObject {
public:
    void Init(const std::string& filename) {
        
    }

    void Info(const std::string& str) {
        std::cout << str << std::endl;
    }
    void Warn(const std::string& str) {
        std::cout << str << std::endl;
    }
    void Err(const std::string& str) {
        std::cout << str << std::endl;
    }
};

inline LogObject& Log() {
    static LogObject log_ = LogObject();
    return log_;
}

#endif
