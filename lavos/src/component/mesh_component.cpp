
#include "lavos/component/mesh_component.h"

lavos::MeshComponent::MeshComponent(lavos::Mesh *mesh)
{
	SetMesh(mesh);
}

void lavos::MeshComponent::BindBuffers(vk::CommandBuffer command_buffer)
{
	command_buffer.bindVertexBuffers(0, { mesh->vertex_buffer.buffer }, { 0 });
	command_buffer.bindIndexBuffer(mesh->index_buffer.buffer, 0, vk::IndexType::eUint16);
}

unsigned int lavos::MeshComponent::GetPrimitivesCount()
{
	return static_cast<unsigned int>(mesh->primitives.size());
}

lavos::Renderable::Primitive *lavos::MeshComponent::GetPrimitive(unsigned int i)
{
	return &mesh->primitives[i];
}
