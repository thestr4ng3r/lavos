
#ifndef LAVOS_MESH_COMPONENT_H
#define LAVOS_MESH_COMPONENT_H

#include "component.h"
#include "../mesh.h"
#include "../renderable.h"

namespace lavos
{

class MeshComponent: public Component, public Renderable
{
	private:
		Mesh *mesh;

	public:
		MeshComponent(Mesh *mesh = nullptr);

		void SetMesh(Mesh *mesh)				{ this->mesh = mesh; }
		Mesh *GetMesh() const 					{ return mesh; }

		bool GetCurrentlyRenderable() const override	{ return mesh != nullptr; }

		void BindBuffers(vk::CommandBuffer command_buffer) override;
		unsigned int GetPrimitivesCount() override;
		Primitive *GetPrimitive(unsigned int i) override;
};

}

#endif //VULKAN_MESH_COMPONENT_H
