
#include "lavos/component/spot_light_component.h"
#include "lavos/spot_light_shadow.h"
#include "lavos/node.h"
#include "lavos/component/transform_component.h"

using namespace lavos;

SpotLightComponent::SpotLightComponent(glm::vec3 intensity, float angle)
		: intensity(intensity), angle(angle)
{
}

SpotLightComponent::~SpotLightComponent()
{
	DestroyShadow();
}

glm::mat4 SpotLightComponent::GetModelViewMatrix()
{
	auto transform_component = GetNode()->GetTransformComponent();
	if(transform_component == nullptr)
		throw std::runtime_error("node with a spot light component does not have a transform component.");

	glm::mat4 transform_mat = transform_component->GetMatrixWorld();
	return glm::inverse(transform_mat);
}

glm::mat4 SpotLightComponent::GetProjectionMatrix(float near_clip, float far_clip)
{
	return glm::perspective(angle, 1.0f, near_clip, far_clip);
}

void SpotLightComponent::InitShadow(Engine *engine, SpotLightShadowRenderer *renderer)
{
	DestroyShadow();
	shadow = new SpotLightShadow(engine, this, renderer);
}

void SpotLightComponent::DestroyShadow()
{
	delete shadow;
}
