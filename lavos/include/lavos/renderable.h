
#ifndef LAVOS_RENDERABLE_COMPONENT_H
#define LAVOS_RENDERABLE_COMPONENT_H

#include "lavos/component/component.h"
#include "lavos/material/material_instance.h"

namespace lavos
{

/**
 * Something that can be rendered by a {@link Renderer}.
 *
 * A Renderable consists of 0 or more {@link Primitive}s.
 * Each Primitive has exactly one {@link MaterialInstance} assigned to it and
 * describes a part of the Renderable. For a Mesh, for example, there would be
 * a Primitive for each part of the mesh that uses a different MaterialInstance.
 */
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

		/**
		 * @return true iff the Renderable can currently be rendered.
		 */
		virtual bool GetCurrentlyRenderable() const		{ return true; }

		/**
		 * Bind Vertex, Index, etc. buffers
		 * @param command_buffer
		 */
		virtual void BindBuffers(vk::CommandBuffer command_buffer) =0;

		virtual unsigned int GetPrimitivesCount() const =0;
		virtual Primitive *GetPrimitive(unsigned int i) const =0;
};

}

#endif //LAVOS_RENDERABLE_COMPONENT_H
