
#ifndef LAVOS_DIRECTIONAL_LIGHT_H
#define LAVOS_DIRECTIONAL_LIGHT_H

#include <glm/ext/vector_float3.hpp>

#include "component.h"

namespace lavos
{

class DirectionalLight: public Component
{
	private:
		glm::vec3 intensity;

	public:
		DirectionalLight(glm::vec3 intensity = glm::vec3(1.0f, 1.0f, 1.0f));
		~DirectionalLight();

		glm::vec3 GetIntensity() const 					{ return intensity; }
		void SetIntensity(const glm::vec3 &intensity)	{ this->intensity = intensity; }
};

}

#endif //VULKAN_DIRECTIONAL_LIGHT_COMPONENT_H
