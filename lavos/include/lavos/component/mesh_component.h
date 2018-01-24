
#ifndef VULKAN_MESH_COMPONENT_H
#define VULKAN_MESH_COMPONENT_H

#include "component.h"
#include "../mesh.h"

namespace lavos
{

class MeshComponent: public Component
{
	private:
		Mesh *mesh;

	public:
		MeshComponent(Mesh *mesh = nullptr);

		void SetMesh(Mesh *mesh)	{ this->mesh = mesh; }
		Mesh *GetMesh() const 		{ return mesh; }
};

}

#endif //VULKAN_MESH_COMPONENT_H
