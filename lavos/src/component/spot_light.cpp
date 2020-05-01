
#include "lavos/component/spot_light.h"
#include "lavos/spot_light_shadow.h"
#include "lavos/node.h"
#include "lavos/component/transform_component.h"

using namespace lavos;

SpotLight::SpotLight(glm::vec3 intensity, float angle)
		: intensity(intensity), angle(angle)
{
}

SpotLight::~SpotLight()
{
	DestroyShadow();
}

void SpotLight::InitShadow(Engine *engine, SpotLightShadowRenderer *renderer, float near_clip, float far_clip)
{
	DestroyShadow();
	shadow = new SpotLightShadow(engine, this, renderer, near_clip, far_clip);
}

void SpotLight::DestroyShadow()
{
	delete shadow;
}
