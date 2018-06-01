
#ifndef LAVOS_SPOT_LIGHT_COMPONENT_H
#define LAVOS_SPOT_LIGHT_COMPONENT_H

#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>

#include "component.h"

namespace lavos
{

class Engine;
class SpotLightShadow;

class SpotLightComponent: public Component
{
	private:
		glm::vec3 intensity;
		float angle;

		SpotLightShadow *shadow = nullptr;

	public:
		SpotLightComponent(glm::vec3 intensity = glm::vec3(1.0f, 1.0f, 1.0f), float angle = glm::half_pi<float>());
		~SpotLightComponent();

		/**
		 * in radians
		 */
		float GetAngle() const 							{ return angle; }

		/**
		 * in radians
		 */
		void SetIntensity(float angle)					{ this->angle = angle; }

		glm::vec3 GetIntensity() const 					{ return intensity; }
		void SetIntensity(const glm::vec3 &intensity)	{ this->intensity = intensity; }

		void InitShadow(Engine *engine, std::uint32_t width, std::uint32_t height);
		void DestroyShadow();
		SpotLightShadow *GetShadow()					{ return shadow; }
};

}

#endif //VULKAN_SPOT_LIGHT_COMPONENT_H
