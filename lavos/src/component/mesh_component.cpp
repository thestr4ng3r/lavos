
#include "lavos/component/mesh_component.h"

lavos::MeshComp::MeshComp(lavos::Mesh *mesh)
{
	SetMesh(mesh);
}

void lavos::MeshComp::BindBuffers(vk::CommandBuffer command_buffer)
{
	command_buffer.bindVertexBuffers(0, { mesh->vertex_buffer->GetVkBuffer() }, { 0 });
	command_buffer.bindIndexBuffer(mesh->index_buffer->GetVkBuffer(), 0, vk::IndexType::eUint16);
}

unsigned int lavos::MeshComp::GetPrimitivesCount()
{
	return static_cast<unsigned int>(mesh->primitives.size());
}

lavos::Renderable::Primitive *lavos::MeshComp::GetPrimitive(unsigned int i)
{
	return &mesh->primitives[i];
}
