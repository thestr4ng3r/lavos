
#ifndef LAVOS_MESH_H
#define LAVOS_MESH_H

#include <string>
#include <vector>
#include <memory>

#include "vertex.h"
#include "buffer.h"
#include "renderable.h"
#include "material/material_instance.h"

namespace lavos
{

class Mesh
{
	private:
		Engine * const engine;

	public:
		struct Primitive: public Renderable::Primitive
		{
			MaterialInstance *material_instance;
			uint32_t indices_count;
			uint32_t indices_offset;

			MaterialInstance *GetMaterialInstance()	override	{ return material_instance; }
			void Draw(vk::CommandBuffer command_buffer) override;
		};

		std::vector<Vertex> vertices;
		std::vector<uint16_t> indices;
		std::vector<Primitive> primitives;

		lavos::Buffer *vertex_buffer = nullptr;
		lavos::Buffer *index_buffer = nullptr;

		Mesh(Engine *engine);
		~Mesh();

		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateBuffers();
};

}

#endif //VULKAN_MESH_H
