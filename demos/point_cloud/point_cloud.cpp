
#if defined(__ANDROID__)
#include <android_common.h>
#endif

#include <iostream>
#include <set>
#include <fstream>
#include <chrono>

#include <lavos/glm_config.h>

#include <lavos/component/point_cloud_component.h>
#include <lavos/material/point_cloud_material.h>
#include <lavos/component/directional_light_component.h>
#include <lavos/component/fp_controller_component.h>
#include <lavos/asset_container.h>
#include <lavos/point_cloud.h>

#include <window_application.h>

#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>


lavos::shell::glfw::WindowApplication *app = nullptr;

lavos::Renderer *renderer = nullptr;
lavos::Material *material;

lavos::PointCloud<glm::vec3> *point_cloud = nullptr;

lavos::MaterialInstance *material_instance;

lavos::Scene *scene;

double last_cursor_x, last_cursor_y;
lavos::FirstPersonControllerComponent *fp_controller;


void Init()
{
	material = new lavos::PointCloudMaterial(app->GetEngine());

	auto render_config = lavos::RenderConfigBuilder().Build();

	renderer = new lavos::Renderer(app->GetEngine(), render_config, app->GetSwapchain(), app->GetDepthRenderTarget());
	renderer->AddMaterial(material);

	scene = new lavos::Scene();

	renderer->SetScene(scene);

	pcl::PointCloud<pcl::PointXYZ>::Ptr pcl_cloud(new pcl::PointCloud<pcl::PointXYZ>);
	if(pcl::io::loadPCDFile("data/lamppost.pcd", *pcl_cloud) == -1)
	{
		throw std::runtime_error("Failed to load point cloud.");
	}

	point_cloud = new lavos::PointCloud<glm::vec3>(app->GetEngine());
	auto point_cloud_size = (*pcl_cloud).size();
	point_cloud->points.resize(point_cloud_size);
	for(size_t i=0; i<point_cloud_size; i++)
	{
		auto &pcl_point = (*pcl_cloud).points[i];
		point_cloud->points[i] = glm::vec3(pcl_point.x, pcl_point.y, pcl_point.z);
	}
	point_cloud->CreateBuffers();

	auto *point_cloud_node = new lavos::Node();
	point_cloud_node->AddComponent(new lavos::TransformComponent());
	scene->GetRootNode()->AddChild(point_cloud_node);

	auto *point_cloud_component = new lavos::PointCloudComponent<glm::vec3>(point_cloud);
	point_cloud_node->AddComponent(point_cloud_component);


	auto *camera_node = new lavos::Node();
	scene->GetRootNode()->AddChild(camera_node);

	camera_node->AddComponent(new lavos::TransformComponent());

	camera_node->GetTransformComponent()->translation = glm::vec3(-8.0f, 5.0f, -3.0f);
	camera_node->GetTransformComponent()->SetLookAt(glm::vec3(-10.0f, 0.0f, -3.0f));

	auto *camera = new lavos::CameraComponent();
	camera->SetNearClip(0.01f);
	camera_node->AddComponent(camera);


	auto *light_node = new lavos::Node();
	scene->GetRootNode()->AddChild(light_node);

	light_node->AddComponent(new lavos::TransformComponent());
	light_node->GetTransformComponent()->SetLookAt(glm::vec3(-1.0f, -1.0f, -1.0f));

	lavos::DirectionalLightComponent *light = new lavos::DirectionalLightComponent();
	light_node->AddComponent(light);


	renderer->SetCamera(camera);
}

void Cleanup()
{
	delete renderer;
	delete scene;
	delete point_cloud;
	delete material;
	delete app;
}

int main(int argc, const char **argv)
{
	app = new lavos::shell::glfw::WindowApplication(800, 600, "Point Cloud", true);

	Init();

	while(true)
	{
		app->BeginFrame();
		app->Update();

		if(glfwWindowShouldClose(app->GetWindow()))
			break;

		app->Render(renderer);
		app->EndFrame();
	}

	Cleanup();

	return EXIT_SUCCESS;
}