
#ifndef VULKAN_ASSET_CONTAINER_H
#define VULKAN_ASSET_CONTAINER_H

#include <string>
#include <map>

#include "material_instance.h"
#include "mesh.h"
#include "renderer.h"

namespace engine
{

class AssetContainer
{
	public:
		Renderer * const renderer;

		std::vector<MaterialInstance *> material_instances;
		std::vector<Mesh *> meshes;

		AssetContainer(Renderer *renderer);
		~AssetContainer();

		static AssetContainer *LoadFromGLTF(Renderer *renderer, std::string filename);
};

}

#endif //VULKAN_GLTF_LOADER_H
