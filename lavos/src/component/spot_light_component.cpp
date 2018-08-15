
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

void SpotLightComponent::InitShadow(Engine *engine, SpotLightShadowRenderer *renderer, float near_clip, float far_clip)
{
	DestroyShadow();
	shadow = new SpotLightShadow(engine, this, renderer, near_clip, far_clip);
}

void SpotLightComponent::DestroyShadow()
{
	delete shadow;
}
