
#include "lavos/light_collection.h"
#include "lavos/scene.h"
#include "lavos/component/directional_light.h"
#include "lavos/component/spot_light.h"

using namespace lavos;

LightCollection LightCollection::EverythingInScene(Scene *scene)
{
	LightCollection ret;
	ret.dir_light = scene->GetRootNode()->GetComponentInChildren<DirectionalLight>();
	ret.spot_lights = scene->GetRootNode()->GetComponentsInChildren<SpotLight>();
	return std::move(ret);
}