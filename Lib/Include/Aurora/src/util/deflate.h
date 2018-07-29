#ifndef DEFLATE_H
#define DEFLATE_H

#include <vector>

#include "zlib/zlib.h"

namespace Au{

std::vector<char> Inflate(const std::vector<char>& data, size_t size_guess = 0);
std::vector<char> Deflate(const std::vector<char>& data);

}

#endif
