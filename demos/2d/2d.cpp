

#include <iostream>
#include <set>
#include <fstream>
#include <chrono>

#include <lavos/glm_config.h>

#include <vulkan/vulkan.h>
#include <lavos/engine.h>
#include <lavos/asset_container.h>
#include <lavos/component/mesh_component.h>
#include <lavos/component/camera_component.h>
#include <lavos/material/phong_material.h>
#include <lavos/material/gouraud_material.h>
#include <lavos/material/unlit_material.h>
#include <lavos/component/directional_light_component.h>
#include <lavos/component/fp_controller_component.h>

#include <window_application.h>



lavosframe::WindowApplication *app = nullptr;

lavos::Renderer *renderer = nullptr;
lavos::Material *material;

lavos::AssetContainer *asset_container = nullptr;

lavos::MaterialInstance *material_instance;

lavos::Scene *scene;


class CameraControllerComponent : public lavos::Component
{
	private:
		glm::vec2 velocity;

	public:
		virtual void Update(float delta_time) override
		{
			lavos::TransformComponent *transform = GetNode()->GetTransformComponent();

			glm::mat4 mat = transform->GetMatrix();
			transform->translation += glm::vec3(velocity.x * delta_time, velocity.y * delta_time, 0.0f);
		}

		void SetVelocity(glm::vec2 velocity)	{ this->velocity = velocity; }
};

CameraControllerComponent *camera_controller;


void Init(std::string gltf_filename)
{
	material = new lavos::GouraudMaterial(app->GetEngine());
	renderer = new lavos::Renderer(app->GetEngine(), app->GetSwapchainExtent(), app->GetSwapchainImageFormat(), app->GetSwapchainImageViews());
	renderer->AddMaterial(material);

	asset_container = lavos::AssetContainer::LoadFromGLTF(app->GetEngine(), material, gltf_filename);

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

	camera->SetType(lavos::CameraComponent::Type::ORTHOGRAPHIC);
	camera->SetOrthographicParams(-2.0, 2.0, -2.0, 2.0);

	camera_node->GetTransformComponent()->translation = glm::vec3(0.0f, 0.0f, 5.0f);
	camera_node->GetTransformComponent()->SetLookAt(glm::vec3(0.0f, 0.0f, 0.0f));

	camera_controller = new CameraControllerComponent();
	camera_node->AddComponent(camera_controller);


	lavos::Node *light_node = new lavos::Node();
	scene->GetRootNode()->AddChild(light_node);

	light_node->AddComponent(new lavos::TransformComponent());
	light_node->GetTransformComponent()->SetLookAt(glm::vec3(-1.0f, -1.0f, -1.0f));

	lavos::DirectionalLightComponent *light = new lavos::DirectionalLightComponent();
	light_node->AddComponent(light);

	renderer->SetCamera(camera);

	material_instance = asset_container->material_instances.front();
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

	camera_controller->SetVelocity(cam_vel * 10.0f);

	scene->Update(app->GetDeltaTime());
}


void Cleanup()
{
	delete asset_container;
	delete renderer;
}

int main(int argc, const char **argv)
{
	std::string gltf_filename = "data/gltftest.gltf";
	if(argc > 1)
		gltf_filename = argv[1];

	app = new lavosframe::WindowApplication(800, 600, "First Person", true);

	Init(gltf_filename);

	while(true)
	{
		app->BeginFrame();
		app->Update();

		Update();

		if(glfwWindowShouldClose(app->GetWindow()))
			break;

		if(app->GetSwapchainRecreated())
			renderer->ResizeScreen(app->GetSwapchainExtent(), app->GetSwapchainImageViews());

		app->Render(renderer);
		app->EndFrame();
	}

	Cleanup();

	return EXIT_SUCCESS;
}