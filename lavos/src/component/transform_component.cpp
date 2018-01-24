
#include "lavos/component/transform_component.h"

using namespace lavos;

void TransformComponent::SetLookAt(glm::vec3 target, glm::vec3 up)
{
	glm::mat4 m = glm::lookAt(translation, target, up);
	m = glm::inverse(m);
	rotation = glm::toQuat(m);
}
