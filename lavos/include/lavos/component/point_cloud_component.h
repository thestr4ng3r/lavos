
#ifndef LAVOS_POINT_CLOUD_COMPONENT_H
#define LAVOS_POINT_CLOUD_COMPONENT_H

#include "component.h"
#include "../point_cloud.h"

namespace lavos
{

template<class Point>
class PointCloudComponent: public Component
{
	private:
		PointCloud<Point> *point_cloud;

	public:
		PointCloudComponent(PointCloud<Point> *point_cloud = nullptr)	{ SetPointCloud(point_cloud); }

		void SetPointCloud(PointCloud<Point> *point_cloud)	{ this->point_cloud = point_cloud; }
		PointCloud<Point> *GetPointCloud() const 			{ return point_cloud; }
};

}

#endif //LAVOS_POINT_CLOUD_COMPONENT_H
