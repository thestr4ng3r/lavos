
#include "shader_load.h"

#include <fstream>

std::vector<char> ReadFile(const std::string &filename)
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

std::vector<char> ReadSPIRVShader(const std::string shader)
{
#if defined(__ANDROID__)
	return AndroidReadSPIRVShader(shader);
#else
	return ReadFile(std::string(std::getenv("SHADER_PATH")) + "/" + shader + ".spv");
#endif
}
