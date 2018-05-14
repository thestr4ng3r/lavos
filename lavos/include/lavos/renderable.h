
#ifndef LAVOS_RENDERABLE_COMPONENT_H
#define LAVOS_RENDERABLE_COMPONENT_H

#include "lavos/component/component.h"
#include "lavos/material/material_instance.h"

namespace lavos
{

class Renderable
{
	public:
		class Primitive
		{
			public:
				virtual ~Primitive() = default;

				virtual MaterialInstance *GetMaterialInstance() =0;
				virtual void Draw(vk::CommandBuffer command_buffer) =0;
		};

		virtual ~Renderable() = default;

		virtual bool GetCurrentlyRenderable() const		{ return true; }

		virtual void BindBuffers(vk::CommandBuffer command_buffer) =0;
		virtual unsigned int GetPrimitivesCount() const =0;
		virtual Primitive *GetPrimitive(unsigned int i) const =0;
};

}

#endif //LAVOS_RENDERABLE_COMPONENT_H
