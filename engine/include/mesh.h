
#ifndef VULKAN_MESH_H
#define VULKAN_MESH_H

#include <string>

namespace engine
{

class Mesh
{
	public:
		Mesh(std::string gltf_file);
		~Mesh();
};

}

#endif //VULKAN_MESH_H
