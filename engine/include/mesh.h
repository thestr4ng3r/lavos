
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
			MaterialInstance *material;
			size_t vertices_offset;
			size_t indices_offset;
		};

		std::vector<Vertex> vertices;
		std::vector<uint16_t> indices;
		std::vector<Primitive> primitives;

		Mesh();
		~Mesh();
};

}

#endif //VULKAN_MESH_H
