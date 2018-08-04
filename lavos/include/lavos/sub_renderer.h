
#ifndef LAVOS_SUB_RENDERER_H
#define LAVOS_SUB_RENDERER_H

namespace lavos
{

class Engine;
class Material;

class SubRenderer
{
	protected:
		Engine * const engine;

	public:
		explicit SubRenderer(Engine *engine);
		virtual ~SubRenderer() = default;

		virtual void AddMaterial(Material *material) {}
		virtual void RemoveMaterial(Material *material) {}
};

}

#endif //LAVOS_SUB_RENDERER_H
