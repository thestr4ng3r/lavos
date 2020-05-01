
#ifndef LAVOS_POINT_CLOUD_COMPONENT_H
#define LAVOS_POINT_CLOUD_COMPONENT_H

#include "component.h"
#include "../renderable.h"
#include "../point_cloud.h"

namespace lavos
{

template<class Point>
class PointCloudComp: public Component, public Renderable, public Renderable::Primitive
{
	private:
		PointCloud<Point> *point_cloud;
		MaterialInstance *material_instance;

	public:
		PointCloudComp(PointCloud<Point> *point_cloud = nullptr)	{ SetPointCloud(point_cloud); }

		void SetPointCloud(PointCloud<Point> *point_cloud)	{ this->point_cloud = point_cloud; }
		PointCloud<Point> *GetPointCloud() const 			{ return point_cloud; }

		bool GetCurrentlyRenderable() const
		{
			return point_cloud != nullptr && material_instance != nullptr;
		}

		void BindBuffers(vk::CommandBuffer command_buffer) override
		{
			command_buffer.bindVertexBuffers(0, { point_cloud->vertex_buffer->GetVkBuffer() }, { 0 });
		}

		unsigned int GetPrimitivesCount() override							{ return 1; }
		Renderable::Primitive *GetPrimitive(unsigned int i) override		{ return this; }

		MaterialInstance *GetMaterialInstance() override					{ return material_instance; }

		void Draw(vk::CommandBuffer command_buffer) override
		{
			command_buffer.draw(static_cast<uint32_t>(point_cloud->points.size()), 1, 0, 0);
		}
};

}

#endif //LAVOS_POINT_CLOUD_COMPONENT_H
