
#ifndef _DEMO_COMMON_SHADER_LOAD_H
#define _DEMO_COMMON_SHADER_LOAD_H

#include <string>
#include <vector>

namespace engine
{
const uint32_t *GetSPIRVShader(const std::string shader, size_t *size);
}

#endif