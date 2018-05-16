
#include "lavos/component/spot_light_component.h"

using namespace lavos;

SpotLightComponent::SpotLightComponent(glm::vec3 intensity, float angle)
		: intensity(intensity), angle(angle)
{
}

SpotLightComponent::~SpotLightComponent()
{
}
