#ifndef DIALOG_WRITE_DATA_H
#define DIALOG_WRITE_DATA_H

#include "has_suffix.h"
#include <external/nativefiledialog/nfd.h>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>

inline void DialogWriteDataToDisk(char* data, size_t sz, const std::string& suffix) {
    char* outPath;
    auto r = NFD_SaveDialog(suffix.c_str(), NULL, &outPath);
    if(r == NFD_OKAY) {
        std::string filePath(outPath);
        if(!has_suffix(filePath, suffix)) {
            filePath = filePath + suffix;
        }
        std::cout << filePath << std::endl;
        std::ofstream f(filePath, std::ios::binary);
        if(f.is_open()) {
            f.write(data, sz);
            f.close();
        }
    }
}

template<typename T>
inline void DialogExportResource(std::shared_ptr<T> res, const std::string& suffix) {
    if(res) {
        std::vector<unsigned char> buf;
        if(res->Serialize(buf)) {
            DialogWriteDataToDisk((char*)buf.data(), buf.size(), suffix);
        }
    }
}

#endif
