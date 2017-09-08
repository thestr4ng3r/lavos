
#ifndef _DEMO_COMMON_SHADER_LOAD_H
#define _DEMO_COMMON_SHADER_LOAD_H

#include <string>
#include <vector>

namespace engine
{
std::vector<char> ReadSPIRVShader(const std::string shader);
}

#endif