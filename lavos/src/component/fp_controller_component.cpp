
#include "component/fp_controller_component.h"

using namespace lavos;

void FirstPersonControllerComponent::Update(float delta_time)
{
	TransformComponent *transform = GetNode()->GetTransformComponent();

	transform->rotation = glm::quat(glm::vec3(-rotation.y, -rotation.x, 0.0f));

	glm::mat4 mat = transform->GetMatrix();
	transform->translation += glm::vec3(mat * glm::vec4(velocity.x * delta_time, 0.0f, -velocity.y * delta_time, 0.0f));
}

void FirstPersonControllerComponent::Rotate(glm::vec2 rot)
{
	rotation += rot;
}

void FirstPersonControllerComponent::SetVelocity(glm::vec2 velocity)
{
	this->velocity = velocity;
}
