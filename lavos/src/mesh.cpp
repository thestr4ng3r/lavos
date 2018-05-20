
#include "lavos/mesh.h"
#include "lavos/engine.h"

#include "mikktspace.h"

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


// Returns the number of faces (triangles/quads) on the mesh to be processed.
static int MikkTSpace_GetNumFaces(const SMikkTSpaceContext *pContext)
{

}

// Returns the number of vertices on face number iFace
// iFace is a number in the range {0, 1, ..., getNumFaces()-1}
static int MikkTSpace_GetNumVerticesOfFace(const SMikkTSpaceContext *pContext, const int iFace)
{

}

// returns the position/normal/texcoord of the referenced face of vertex number iVert.
// iVert is in the range {0,1,2} for triangles and {0,1,2,3} for quads.
static void MikkTSpace_GetPosition(const SMikkTSpaceContext * pContext, float fvPosOut[], const int iFace, const int iVert)
{

}

static void MikkTSpace_GetNormal(const SMikkTSpaceContext * pContext, float fvNormOut[], const int iFace, const int iVert)
{

}

static void MikkTSpace_GetTexCoord(const SMikkTSpaceContext * pContext, float fvTexcOut[], const int iFace, const int iVert)
{

}

// either (or both) of the two setTSpace callbacks can be set.
// The call-back m_setTSpaceBasic() is sufficient for basic normal mapping.

// This function is used to return the tangent and fSign to the application.
// fvTangent is a unit length vector.
// For normal maps it is sufficient to use the following simplified version of the bitangent which is generated at pixel/vertex level.
// bitangent = fSign * cross(vN, tangent);
// Note that the results are returned unindexed. It is possible to generate a new index list
// But averaging/overwriting tangent spaces by using an already existing index list WILL produce INCRORRECT results.
// DO NOT! use an already existing index list.
static void MikkTSpace_SetTSpaceBasic(const SMikkTSpaceContext * pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
{

}

// This function is used to return tangent space results to the application.
// fvTangent and fvBiTangent are unit length vectors and fMagS and fMagT are their
// true magnitudes which can be used for relief mapping effects.
// fvBiTangent is the "real" bitangent and thus may not be perpendicular to fvTangent.
// However, both are perpendicular to the vertex normal.
// For normal maps it is sufficient to use the following simplified version of the bitangent which is generated at pixel/vertex level.
// fSign = bIsOrientationPreserving ? 1.0f : (-1.0f);
// bitangent = fSign * cross(vN, tangent);
// Note that the results are returned unindexed. It is possible to generate a new index list
// But averaging/overwriting tangent spaces by using an already existing index list WILL produce INCRORRECT results.
// DO NOT! use an already existing index list.
static void MikkTSpace_SetTSpace(const SMikkTSpaceContext * pContext, const float fvTangent[], const float fvBiTangent[], const float fMagS, const float fMagT,
								 const tbool bIsOrientationPreserving, const int iFace, const int iVert)
{

}

static void

void Mesh::GenerateTangentSpaceMikkTSpace(Mesh::Primitive *primitive)
{
	SMikkTSpaceInterface interface;
	interface.

			SMikkTSpaceContext context;
	context.m_pInterface = &interface;
	context.m_pUserData = this;

	genTangSpaceDefault(&context);
}
