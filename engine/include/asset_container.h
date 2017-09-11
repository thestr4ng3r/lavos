
#ifndef VULKAN_ASSET_CONTAINER_H
#define VULKAN_ASSET_CONTAINER_H

#include <string>
#include <map>

#include "material_instance.h"
#include "mesh.h"
#include "scene.h"

namespace engine
{

class AssetContainer
{
	public:
		Engine * const engine;
		vk::DescriptorPool descriptor_pool;

		std::vector<MaterialInstance *> material_instances;
		std::vector<Mesh *> meshes;
		std::vector<Scene *> scenes;

		AssetContainer(Engine *engine);
		~AssetContainer();

		static AssetContainer *LoadFromGLTF(Engine *engine, Material *material, std::string filename);
};

}

#endif //VULKAN_GLTF_LOADER_H
