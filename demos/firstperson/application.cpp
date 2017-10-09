
#if defined(__ANDROID__)
#include <android_common.h>
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

void Application::InitWindow()
{
	DemoApplication::InitWindow();

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetCursorPos(window, &last_cursor_x, &last_cursor_y);
}

void Application::InitVulkan()
{
	DemoApplication::InitVulkan();

	material = new lavos::PhongMaterial(engine);
	renderer = new lavos::Renderer(engine, swapchain_extent, swapchain_image_format, swapchain_image_views);
	renderer->AddMaterial(material);

	asset_container = lavos::AssetContainer::LoadFromGLTF(engine, material, gltf_filename);

	scene = asset_container->scenes[0];
	scene->SetAmbientLightIntensity(glm::vec3(0.3f, 0.3f, 0.3f));

	renderer->SetScene(scene);

	lavos::CameraComponent *camera = scene->GetRootNode()->GetComponentInChildren<lavos::CameraComponent>();
	lavos::Node *camera_node;
	if(camera == nullptr)
	{
		camera_node = new lavos::Node();
		scene->GetRootNode()->AddChild(camera_node);
		camera_node->AddComponent(new lavos::TransformComponent());

		camera = new lavos::CameraComponent();
		camera->SetNearClip(0.01f);
		camera_node->AddComponent(camera);
	}
	else
	{
		camera_node = camera->GetNode();
	}

	camera_node->GetTransformComponent()->translation = glm::vec3(0.0f, 0.0f, 5.0f);
	camera_node->GetTransformComponent()->SetLookAt(glm::vec3(0.0f, 0.0f, 0.0f));

	fp_controller = new lavos::FirstPersonControllerComponent();
	camera_node->AddComponent(fp_controller);


	lavos::Node *light_node = new lavos::Node();
	scene->GetRootNode()->AddChild(light_node);

	light_node->AddComponent(new lavos::TransformComponent());
	light_node->GetTransformComponent()->SetLookAt(glm::vec3(-1.0f, -1.0f, -1.0f));

	lavos::DirectionalLightComponent *light = new lavos::DirectionalLightComponent();
	light_node->AddComponent(light);

	renderer->SetCamera(camera);

	material_instance = asset_container->material_instances.front();
}

void Application::Update(float delta_time)
{
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	glm::vec2 cam_vel(0.0f);

	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cam_vel.y += 1.0f;

	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cam_vel.y -= 1.0f;

	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cam_vel.x -= 1.0f;

	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cam_vel.x += 1.0f;

	fp_controller->SetVelocity(cam_vel * 10.0f);

	double cursor_x, cursor_y;
	glfwGetCursorPos(window, &cursor_x, &cursor_y);
	fp_controller->Rotate(glm::vec2(cursor_x - last_cursor_x, cursor_y - last_cursor_y) * 0.003f);
	last_cursor_x = cursor_x;
	last_cursor_y = cursor_y;

	scene->Update(delta_time);
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
	std::string gltf_filename = "data/gltftest_nocamera.gltf";
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
#else
void sample_main()
{
	Application demo_app("/storage/emulated/0/vulkan/flux/gltftest.gltf");

	try
	{
		demo_app.Run();
	}
	catch(const std::runtime_error &e)
	{
		LOGE("%s\n", e.what());
	}
}
#endif
