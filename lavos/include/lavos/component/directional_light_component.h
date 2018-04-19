
#ifndef LAVOS_DIRECTIONAL_LIGHT_COMPONENT_H
#define LAVOS_DIRECTIONAL_LIGHT_COMPONENT_H

#include <glm/vec3.hpp>

#include "component.h"

namespace lavos
{

class DirectionalLightComponent: public Component
{
	private:
		glm::vec3 intensity;

	public:
		DirectionalLightComponent(glm::vec3 intensity = glm::vec3(1.0f, 1.0f, 1.0f));
		~DirectionalLightComponent();

		glm::vec3 GetIntensity() const 					{ return intensity; }
		void SetIntensity(const glm::vec3 &intensity)	{ this->intensity = intensity; }
};

}

#endif //VULKAN_DIRECTIONAL_LIGHT_COMPONENT_H
