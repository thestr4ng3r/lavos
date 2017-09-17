
#ifndef VULKAN_SCENE_H
#define VULKAN_SCENE_H

#include "glm_config.h"

#include "node.h"

namespace engine
{

class Scene
{
	private:
		Node root_node;

		glm::vec3 ambient_light_intensity;

	public:
		Scene();
		~Scene();

		Node *GetRootNode()									{ return &root_node; }

		glm::vec3 GetAmbientLightIntensity() const 			{ return ambient_light_intensity; }
		void SetAmbientLightIntensity(glm::vec3 intensity)	{ ambient_light_intensity = intensity; }
};


}

#endif //VULKAN_SCENE_H
