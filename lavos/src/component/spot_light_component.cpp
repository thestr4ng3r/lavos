
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

void SpotLightComponent::InitShadow(Engine *engine, std::uint32_t width, std::uint32_t height)
{
	DestroyShadow();
	shadow = new SpotLightShadow(engine, this, width, height);
}

void SpotLightComponent::DestroyShadow()
{
	delete shadow;
}
