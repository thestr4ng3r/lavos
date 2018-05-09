
#include <lavos/engine.h>
#include <lavos/component/directional_light_component.h>
#include <lavos/material/phong_material.h>
#include <lavos/renderer.h>
#include <lavos/asset_container.h>

#include "mainwindow.h"

void MainWindowRenderer::initResources()
{
	engine->InitializeWithDevice(window->physicalDevice(),
								 window->device(),
								 window->graphicsQueue(),
								 window->graphicsQueue()); // TODO: different queue for present?



}

void MainWindowRenderer::initSwapChainResources()
{
	material = new lavos::PhongMaterial(engine);

	render_target = new QVulkanWindowRenderTarget(window);

	renderer = new lavos::Renderer(engine, render_target);
	renderer->AddMaterial(material);

	asset_container = lavos::AssetContainer::LoadFromGLTF(engine, material, "data/gltftest.gltf");

	lavos::Scene *scene = asset_container->scenes[0];
	scene->SetAmbientLightIntensity(glm::vec3(0.3f, 0.3f, 0.3f));

	renderer->SetScene(scene);

	lavos::CameraComponent *camera = scene->GetRootNode()->GetComponentInChildren<lavos::CameraComponent>();

	if(camera == nullptr)
	{
		lavos::Node *camera_node = new lavos::Node();
		scene->GetRootNode()->AddChild(camera_node);

		camera_node->AddComponent(new lavos::TransformComponent());

		camera_node->GetTransformComponent()->translation = glm::vec3(5.0f, 5.0f, 5.0f);
		camera_node->GetTransformComponent()->SetLookAt(glm::vec3(0.0f, 0.0f, 0.0f));

		camera = new lavos::CameraComponent();
		camera->SetNearClip(0.01f);
		camera_node->AddComponent(camera);
	}

	lavos::Node *light_node = new lavos::Node();
	scene->GetRootNode()->AddChild(light_node);

	light_node->AddComponent(new lavos::TransformComponent());
	light_node->GetTransformComponent()->SetLookAt(glm::vec3(-1.0f, -1.0f, -1.0f));

	lavos::DirectionalLightComponent *light = new lavos::DirectionalLightComponent();
	light_node->AddComponent(light);

	renderer->SetCamera(camera);

	//material_instance = asset_container->material_instances.front();
}

void MainWindowRenderer::releaseSwapChainResources()
{
}

void MainWindowRenderer::releaseResources()
{
	// TODO: delete engine?
}

void MainWindowRenderer::startNextFrame()
{
	renderer->DrawFrameRecord(window->currentCommandBuffer(), window->currentFramebuffer());
	window->frameReady();
}

