
#ifndef LAVOS_SPOT_LIGHT_COMPONENT_H
#define LAVOS_SPOT_LIGHT_COMPONENT_H

#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>

#include "component.h"

namespace lavos
{

class SpotLightComponent: public Component
{
	private:
		glm::vec3 intensity;
		float angle;

	public:
		SpotLightComponent(glm::vec3 intensity = glm::vec3(1.0f, 1.0f, 1.0f), float angle = glm::half_pi<float>());
		~SpotLightComponent();

		glm::vec3 GetIntensity() const 					{ return intensity; }
		void SetIntensity(const glm::vec3 &intensity)	{ this->intensity = intensity; }
};

}

#endif //VULKAN_SPOT_LIGHT_COMPONENT_H
