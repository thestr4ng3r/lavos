
#ifndef LAVOS_LIGHT_COLLECTION_H
#define LAVOS_LIGHT_COLLECTION_H

#include <vector>

namespace lavos
{

class DirectionalLight;
class SpotLight;
class Scene;

/**
 * Accumulates all the lights used for rendering,
 * could also be local to a cluster for example.
 */
struct LightCollection
{
	DirectionalLight *dir_light;
	std::vector<SpotLight *> spot_lights;

	static LightCollection EverythingInScene(Scene *scene);
};

}

#endif //LAVOS_LIGHT_COLLECTION_H
