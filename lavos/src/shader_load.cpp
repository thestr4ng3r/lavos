
#include "lavos/shader_load.h"

#include <fstream>
#include <stdexcept>

#include "spirv_resources.h"

using namespace lavos;

#if defined(__ANDROID__)
#include <android_common.h>
#endif

static std::vector<char> ReadFile(const std::string &filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if(!file.is_open())
		throw std::runtime_error("failed to open file!");

	size_t size = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(size);

	file.seekg(0);
	file.read(buffer.data(), size);
	file.close();

	return buffer;
}

/*std::vector<char> engine::ReadSPIRVShader(const std::string shader)
{
#if defined(__ANDROID__)
	return AndroidReadSPIRVShader(shader);
#else
	const char *shader_path = std::getenv("SHADER_PATH");
	if(!shader_path)
		shader_path = ".";

	return ReadFile(std::string(shader_path) + "/" + shader + ".spv");
#endif
}*/


const uint32_t *lavos::GetSPIRVShader(const std::string shader, size_t *size)
{
	std::string filename = shader + ".spv";
	return reinterpret_cast<const uint32_t *>(spirv_resources_get(filename.c_str(), size));
}