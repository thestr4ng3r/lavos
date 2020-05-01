
#ifndef LAVOS_SPOT_LIGHT_H
#define LAVOS_SPOT_LIGHT_H

#include "../glm_config.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/constants.hpp>

#include "component.h"

namespace lavos
{

class Engine;
class SpotLightShadow;
class SpotLightShadowRenderer;

class SpotLight: public Component
{
	private:
		glm::vec3 intensity;
		float angle;

		SpotLightShadow *shadow = nullptr;

	public:
		SpotLight(glm::vec3 intensity = glm::vec3(1.0f, 1.0f, 1.0f), float angle = glm::half_pi<float>());
		~SpotLight();

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

		void InitShadow(Engine *engine, SpotLightShadowRenderer *renderer, float near_clip = 0.1f, float far_clip = 100.0f);
		void DestroyShadow();
		SpotLightShadow *GetShadow()					{ return shadow; }
};

}

#endif //VULKAN_SPOT_LIGHT_COMPONENT_H
