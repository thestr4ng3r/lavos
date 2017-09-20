
#if defined(__ANDROID__)
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <common.h>
#endif

#include <iostream>
#include <set>
#include <fstream>
#include <chrono>

#include <glm_config.h>

#include "application.h"

#include <vulkan/vulkan.h>
#include <engine.h>
#include <asset_container.h>
#include <component/mesh_component.h>
#include <component/camera_component.h>
#include <material/phong_material.h>
#include <material/unlit_material.h>
#include <component/directional_light_component.h>


Application::Application(std::string gltf_filename)
{
	this->gltf_filename = gltf_filename;
}

void Application::InitVulkan()
{
	DemoApplication::InitVulkan();

	material = new engine::PhongMaterial(engine);
	renderer = new engine::Renderer(engine, swapchain_extent, swapchain_image_format, swapchain_image_views);
	renderer->AddMaterial(material);

	asset_container = engine::AssetContainer::LoadFromGLTF(engine, material, gltf_filename);

	engine::Scene *scene = asset_container->scenes[0];
	scene->SetAmbientLightIntensity(glm::vec3(0.3f, 0.3f, 0.3f));

	renderer->SetScene(scene);

	engine::CameraComponent *camera = scene->GetRootNode()->GetComponentInChildren<engine::CameraComponent>();

	if(camera == nullptr)
	{
		engine::Node *camera_node = new engine::Node();
		scene->GetRootNode()->AddChild(camera_node);

		camera_node->AddComponent(new engine::TransformComponent());

		camera_node->GetTransformComponent()->translation = glm::vec3(5.0f, 5.0f, 5.0f);
		camera_node->GetTransformComponent()->SetLookAt(glm::vec3(0.0f, 0.0f, 0.0f));

		camera = new engine::CameraComponent();
		camera->SetNearClip(0.01f);
		camera_node->AddComponent(camera);
	}

	engine::Node *light_node = new engine::Node();
	scene->GetRootNode()->AddChild(light_node);

	light_node->AddComponent(new engine::TransformComponent());
	light_node->GetTransformComponent()->SetLookAt(glm::vec3(-1.0f, -1.0f, -1.0f));

	engine::DirectionalLightComponent *light = new engine::DirectionalLightComponent();
	light_node->AddComponent(light);

	renderer->SetCamera(camera);

	material_instance = asset_container->material_instances.front();
}

void Application::DrawFrame(uint32_t image_index)
{
	renderer->DrawFrame(image_index,
						{ image_available_semaphore },
						{ vk::PipelineStageFlagBits::eColorAttachmentOutput },
						{ render_finished_semaphore });
}

void Application::RecreateSwapchain()
{
	DemoApplication::RecreateSwapchain();
	renderer->ResizeScreen(swapchain_extent, swapchain_image_views);
}

void Application::CleanupApplication()
{
	delete asset_container;
	delete renderer;
}


#ifndef __ANDROID__
int main(int argc, const char **argv)
{
	std::string gltf_filename = "data/gltftest.gltf";
	if(argc > 1)
		gltf_filename = argv[1];

	Application app(gltf_filename);

	try
	{
		app.Run();
	}
	catch(const std::runtime_error &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
#endif