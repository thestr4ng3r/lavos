
#include "mesh.h"

#include <tiny_gltf.h>

using namespace engine;

Mesh::Mesh(std::string gltf_file)
{
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	std::string error;

	bool success = loader.LoadASCIIFromFile(&model, &error, gltf_file);

	if(!error.empty())
		throw std::runtime_error(error);

	if(!success)
		throw std::runtime_error("Failed to load glTF file.");
}

Mesh::~Mesh()
{

}
