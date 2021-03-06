
#include "lavos/component/fp_controller.h"

using namespace lavos;

void FirstPersonController::Update(float delta_time)
{
	TransformComp *transform = GetNode()->GetTransformComp();

	transform->rotation = glm::quat(glm::vec3(-rotation.y, -rotation.x, 0.0f));

	glm::mat4 mat = transform->GetMatrix();
	transform->translation += glm::vec3(mat * glm::vec4(velocity.x * delta_time, 0.0f, -velocity.y * delta_time, 0.0f));
}

void FirstPersonController::Rotate(glm::vec2 rot)
{
	rotation += rot;
}

void FirstPersonController::SetVelocity(glm::vec2 velocity)
{
	this->velocity = velocity;
}
