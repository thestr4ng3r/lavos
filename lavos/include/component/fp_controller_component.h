
#ifndef VULKAN_FP_CAMERA_CONTROLLER_COMPONENT_H
#define VULKAN_FP_CAMERA_CONTROLLER_COMPONENT_H

#include "component.h"
#include "transform_component.h"

namespace lavos
{

class FirstPersonControllerComponent : public Component
{
	private:
		glm::vec2 rotation;
		glm::vec2 velocity;

	public:
		virtual void Update(float delta_time) override;

		void Rotate(glm::vec2 rot);
		void SetVelocity(glm::vec2 velocity);
};

}

#endif //VULKAN_FP_CAMERA_CONTROLLER_COMPONENT_H
