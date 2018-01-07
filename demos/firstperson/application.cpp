
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

Application::Application(std::string gltf_filename) : app(800, 600, "GLTF", true)
{
	this->gltf_filename = gltf_filename;


	material = new lavos::PhongMaterial(app.GetEngine());
	renderer = new lavos::Renderer(app.GetEngine(), app.GetSwapchainExtent(), app.GetSwapchainImageFormat(), app.GetSwapchainImageViews());
	renderer->AddMaterial(material);

	asset_container = lavos::AssetContainer::LoadFromGLTF(app.GetEngine(), material, gltf_filename);

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






	GLFWwindow *window = app.GetWindow();
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetCursorPos(window, &last_cursor_x, &last_cursor_y);
}


Application::~Application()
{
	delete asset_container;
	delete renderer;
}

void Application::Run()
{
	while(true)
	{
		app.BeginFrame();

		app.Update();

		Update(app.GetDeltaTime());

		if(glfwWindowShouldClose(app.GetWindow()))
			break;

		if(app.GetSwapchainRecreated())
			renderer->ResizeScreen(app.GetSwapchainExtent(), app.GetSwapchainImageViews());

		app.Render(renderer);

		app.EndFrame();
	}
}

void Application::Update(float delta_time)
{
	GLFWwindow *window = app.GetWindow();

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
