
#ifndef VULKAN_TRANSFORM_COMPONENT_H
#define VULKAN_TRANSFORM_COMPONENT_H

#include "component.h"

#include "glm_config.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine
{

class TransformComponent: public Component
{
	public:
		glm::vec3 translation = glm::vec3(0.0f);
		glm::quat rotation;
		glm::vec3 scale = glm::vec3(1.0f);

		glm::mat4 GetMatrix()
		{
			return glm::scale(glm::translate(glm::mat4(1.0f), translation), scale) * glm::toMat4(rotation);
		}
};

}

#endif //VULKAN_TRANSFORM_COMPONENT_H
