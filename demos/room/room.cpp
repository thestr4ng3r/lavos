

#include <iostream>
#include <set>
#include <fstream>
#include <chrono>

#include <lavos/glm_config.h>

#include <vulkan/vulkan.h>
#include <lavos/engine.h>
#include <lavos/render_config.h>
#include <lavos/asset_container.h>
#include <lavos/component/mesh_component.h>
#include <lavos/component/camera_component.h>
#include <lavos/material/phong_material.h>
#include <lavos/material/gouraud_material.h>
#include <lavos/material/unlit_material.h>
#include <lavos/component/directional_light_component.h>
#include <lavos/component/spot_light_component.h>
#include <lavos/component/fp_controller_component.h>

#include <window_application.h>



lavos::shell::glfw::WindowApplication *app = nullptr;

lavos::Renderer *renderer = nullptr;
lavos::Material *material;

lavos::AssetContainer *asset_container = nullptr;

lavos::MaterialInstance *material_instance;

lavos::Scene *scene;

bool mouse_caught;
double last_cursor_x, last_cursor_y;
lavos::FirstPersonControllerComponent *fp_controller;


void UpdateMouseSettings();
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void Init(std::string gltf_filename)
{
	material = new lavos::PhongMaterial(app->GetEngine());

	auto render_config = lavos::RenderConfigBuilder().Build();

	renderer = new lavos::Renderer(app->GetEngine(), render_config, app->GetSwapchain(), app->GetDepthRenderTarget());
	renderer->AddMaterial(material);

	asset_container = lavos::AssetContainer::LoadFromGLTF(app->GetEngine(), render_config, material, gltf_filename);

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

	camera_node->GetTransformComponent()->translation = glm::vec3(0.0f, 1.0f, 0.0f);
	camera_node->GetTransformComponent()->SetLookAt(glm::vec3(0.0f, 0.0f, 0.0f));

	fp_controller = new lavos::FirstPersonControllerComponent();
	camera_node->AddComponent(fp_controller);


	lavos::Node *light_node = new lavos::Node();
	scene->GetRootNode()->AddChild(light_node);

	light_node->AddComponent(new lavos::TransformComponent());
	light_node->GetTransformComponent()->translation = glm::vec3(0.0f, 1.0f, 3.0f);
	light_node->GetTransformComponent()->SetLookAt(glm::vec3(2.0f, -1.0f, 0.0f));

	//lavos::DirectionalLightComponent *light = new lavos::DirectionalLightComponent();
	//light_node->AddComponent(light);
	lavos::SpotLightComponent *light = new lavos::SpotLightComponent();
	light_node->AddComponent(light);

	renderer->SetCamera(camera);

	material_instance = asset_container->material_instances.front();


	mouse_caught = false;
	UpdateMouseSettings();

	glfwSetKeyCallback(app->GetWindow(), KeyCallback);
}


void UpdateMouseSettings()
{
	if(mouse_caught)
	{
		glfwSetInputMode(app->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwGetCursorPos(app->GetWindow(), &last_cursor_x, &last_cursor_y);
	}
	else
	{
		glfwSetInputMode(app->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		mouse_caught = !mouse_caught;
		UpdateMouseSettings();
	}
}



void Update()
{
	GLFWwindow *window = app->GetWindow();

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

	if(mouse_caught)
	{
		double cursor_x, cursor_y;
		glfwGetCursorPos(window, &cursor_x, &cursor_y);
		fp_controller->Rotate(glm::vec2(cursor_x - last_cursor_x, cursor_y - last_cursor_y) * 0.003f);
		last_cursor_x = cursor_x;
		last_cursor_y = cursor_y;
	}

	scene->Update(app->GetDeltaTime());
}


void Cleanup()
{
	delete renderer;
	delete asset_container;
	delete material;
}

int main(int argc, const char **argv)
{
	std::string gltf_filename = "data/room.gltf";
	if(argc > 1)
		gltf_filename = argv[1];

	app = new lavos::shell::glfw::WindowApplication(800, 600, "First Person", true);

	Init(gltf_filename);

	while(true)
	{
		app->BeginFrame();
		app->Update();

		Update();

		if(glfwWindowShouldClose(app->GetWindow()))
			break;

		app->Render(renderer);
		app->EndFrame();
	}

	Cleanup();
	delete app;

	return EXIT_SUCCESS;
}