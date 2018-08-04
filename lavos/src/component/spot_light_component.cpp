
#include "lavos/component/spot_light_component.h"
#include "lavos/spot_light_shadow.h"

using namespace lavos;

SpotLightComponent::SpotLightComponent(glm::vec3 intensity, float angle)
		: intensity(intensity), angle(angle)
{
}

SpotLightComponent::~SpotLightComponent()
{
	DestroyShadow();
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
