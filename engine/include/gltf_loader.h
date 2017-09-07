
#ifndef VULKAN_GLTF_LOADER_H
#define VULKAN_GLTF_LOADER_H

#include <string>

#include "material_instance.h"
#include "mesh.h"
#include "renderer.h"

namespace tinygltf
{
class Model;
}

namespace engine
{

class GLTF
{
	private:
		const Renderer *renderer;

		std::string root_path;

		std::vector<MaterialInstance *> material_instances;
		std::vector<Mesh *> meshes;

		void LoadGLTF(tinygltf::Model &model);
		void LoadMaterialInstances(tinygltf::Model &model);
		void LoadMeshes(tinygltf::Model &model);

	public:
		GLTF(Renderer *renderer, std::string filename);
		~GLTF();

		std::vector<MaterialInstance *> &GetMaterialInstances()		{ return material_instances; }
		std::vector<Mesh *> &GetMeshes() 							{ return meshes; };
};

}

#endif //VULKAN_GLTF_LOADER_H
