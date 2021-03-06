
#if defined(__ANDROID__)
#include <android_common.h>
#endif

#include <iostream>
#include <set>
#include <fstream>
#include <chrono>

#include <lavos/glm_config.h>

#include <lavos/component/mesh_component.h>
#include <lavos/material/phong_material.h>
#include <lavos/material/unlit_material.h>
#include <lavos/component/directional_light.h>
#include <lavos/component/fp_controller.h>
#include <lavos/asset_container.h>

#include <window_application.h>


lavos::shell::glfw::WindowApplication *app = nullptr;

lavos::Renderer *renderer = nullptr;
lavos::Material *material;

lavos::AssetContainer *asset_container = nullptr;

lavos::MaterialInstance *material_instance;

lavos::Scene *scene;

double last_cursor_x, last_cursor_y;
lavos::FirstPersonController *fp_controller;


void Init(std::string gltf_filename)
{
	material = new lavos::PhongMaterial(app->GetEngine());

	auto render_config = lavos::RenderConfigBuilder().Build();

	renderer = new lavos::Renderer(app->GetEngine(), render_config, app->GetSwapchain(), app->GetDepthRenderTarget());
	renderer->AddMaterial(material);

	asset_container = lavos::AssetContainer::LoadFromGLTF(app->GetEngine(), render_config, material, gltf_filename);

	lavos::Scene *scene = asset_container->scenes[0];
	scene->SetAmbientLightIntensity(glm::vec3(0.3f, 0.3f, 0.3f));

	renderer->SetScene(scene);

	lavos::Camera *camera = scene->GetRootNode()->GetComponentInChildren<lavos::Camera>();

	if(camera == nullptr)
	{
		lavos::Node *camera_node = new lavos::Node();
		scene->GetRootNode()->AddChild(camera_node);

		camera_node->AddComponent(new lavos::TransformComp());

		camera_node->GetTransformComp()->translation = glm::vec3(5.0f, 5.0f, 5.0f);
		camera_node->GetTransformComp()->SetLookAt(glm::vec3(0.0f, 0.0f, 0.0f));

		camera = new lavos::Camera();
		camera->SetNearClip(0.01f);
		camera_node->AddComponent(camera);
	}

	lavos::Node *light_node = new lavos::Node();
	scene->GetRootNode()->AddChild(light_node);

	light_node->AddComponent(new lavos::TransformComp());
	light_node->GetTransformComp()->SetLookAt(glm::vec3(-1.0f, -1.0f, -1.0f));

	lavos::DirectionalLight *light = new lavos::DirectionalLight();
	light_node->AddComponent(light);

	renderer->SetCamera(camera);

	material_instance = asset_container->material_instances.front();
}

void Cleanup()
{
	delete asset_container;
	delete renderer;
	delete material;
	delete app;
}

int main(int argc, const char **argv)
{
	std::string gltf_filename = "data/gltftest.gltf";
	if(argc > 1)
		gltf_filename = argv[1];

	app = new lavos::shell::glfw::WindowApplication(800, 600, "GLTF", true);

	Init(gltf_filename);

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