
#ifndef VULKAN_MESH_H
#define VULKAN_MESH_H

#include <string>
#include <vector>
#include "vertex.h"
#include "material_instance.h"

namespace engine
{

class Mesh
{
	public:
		struct Primitive
		{
			MaterialInstance *material_instance;
			uint32_t indices_count;
			uint32_t indices_offset;
		};

		std::vector<Vertex> vertices;
		std::vector<uint16_t> indices;
		std::vector<Primitive> primitives;

		Mesh();
		~Mesh();
};

}

#endif //VULKAN_MESH_H
