
#include "shader_load.h"

#include <fstream>

using namespace engine;

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

std::vector<char> engine::ReadSPIRVShader(const std::string shader)
{
#if defined(__ANDROID__)
	return AndroidReadSPIRVShader(shader);
#else
	const char *shader_path = std::getenv("SHADER_PATH");
	if(!shader_path)
		shader_path = ".";

	return ReadFile(std::string(shader_path) + "/" + shader + ".spv");
#endif
}
