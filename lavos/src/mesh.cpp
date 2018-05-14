
#include "lavos/mesh.h"
#include "lavos/engine.h"

#include <iostream>

using namespace lavos;

Mesh::Mesh(Engine *engine)
	: engine(engine)
{
}

Mesh::~Mesh()
{
	engine->DestroyBuffer(index_buffer);
	engine->DestroyBuffer(vertex_buffer);
}

void Mesh::CreateVertexBuffer()
{
	vk::DeviceSize size = sizeof(vertices[0]) * vertices.size();

	auto staging_buffer = engine->CreateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY);
	void *data = engine->MapMemory(staging_buffer.allocation);
	memcpy(data, vertices.data(), size);
	engine->UnmapMemory(staging_buffer.allocation);

	vertex_buffer = engine->CreateBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
										 VMA_MEMORY_USAGE_GPU_ONLY);

	engine->CopyBuffer(staging_buffer.buffer, vertex_buffer.buffer, size);

	engine->DestroyBuffer(staging_buffer);
}

void Mesh::CreateIndexBuffer()
{
	vk::DeviceSize size = sizeof(indices[0]) * indices.size();

	auto staging_buffer = engine->CreateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY);
	void *data = engine->MapMemory(staging_buffer.allocation);
	memcpy(data, indices.data(), size);
	engine->UnmapMemory(staging_buffer.allocation);

	index_buffer = engine->CreateBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
										VMA_MEMORY_USAGE_GPU_ONLY);

	engine->CopyBuffer(staging_buffer.buffer, index_buffer.buffer, size);

	engine->DestroyBuffer(staging_buffer);
}

void Mesh::CreateBuffers()
{
	CreateVertexBuffer();
	CreateIndexBuffer();
}

void Mesh::Primitive::Draw(vk::CommandBuffer command_buffer)
{
	command_buffer.drawIndexed(indices_count, 1, indices_offset, 0, 0);
}
