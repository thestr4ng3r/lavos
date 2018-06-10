
#ifndef LAVOS_POINT_CLOUD_H
#define LAVOS_POINT_CLOUD_H

#include <string>
#include <vector>

#include "engine.h"
#include "vertex.h"
#include "buffer.h"
#include "material/material_instance.h"

namespace lavos
{

template<class Point>
class PointCloud
{
	private:
		Engine * const engine;

	public:
		std::vector<Point> points;

		lavos::Buffer *vertex_buffer = nullptr;

		PointCloud(Engine *engine);
		~PointCloud();

		void CreateVertexBuffer();
		void CreateBuffers();
};


template<class Point>
inline PointCloud<Point>::PointCloud(Engine *engine)
		: engine(engine)
{
}

template<class Point>
inline PointCloud<Point>::~PointCloud()
{
	delete vertex_buffer;
}

template<class Point>
inline void PointCloud<Point>::CreateVertexBuffer()
{
	vk::DeviceSize size = sizeof(points[0]) * points.size();

	auto staging_buffer = engine->CreateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY);
	memcpy(staging_buffer->Map(), points.data(), size);
	staging_buffer->UnMap();

	vertex_buffer = engine->CreateBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
										 VMA_MEMORY_USAGE_GPU_ONLY);

	engine->CopyBuffer(staging_buffer->GetVkBuffer(), vertex_buffer->GetVkBuffer(), size);

	delete staging_buffer;
}

template<class Point>
inline void PointCloud<Point>::CreateBuffers()
{
	CreateVertexBuffer();
}


}


#endif //LAVOS_POINT_CLOUD_H
