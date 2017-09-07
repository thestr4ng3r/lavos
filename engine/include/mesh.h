
#ifndef VULKAN_MESH_H
#define VULKAN_MESH_H

#include <string>
#include <vector>
#include "vertex.h"

namespace engine
{

class Mesh
{
	public:
		std::vector<Vertex> vertices;
		std::vector<uint16_t> indices;

	public:
		Mesh(std::string gltf_file);
		~Mesh();
};

}

#endif //VULKAN_MESH_H
