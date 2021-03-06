
#ifndef LAVOS_SCENE_H
#define LAVOS_SCENE_H

#include "glm_config.h"
#include <glm/ext/vector_float3.hpp>

#include "node.h"

namespace lavos
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

		void Update(float delta_time)						{ root_node.Update(delta_time); }
};


}

#endif //VULKAN_SCENE_H
