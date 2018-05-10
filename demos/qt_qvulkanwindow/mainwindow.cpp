
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
								 window->graphicsQueue());

	material = new lavos::PhongMaterial(engine);

	asset_container = lavos::AssetContainer::LoadFromGLTF(engine, material, "data/gltftest.gltf");

	scene = asset_container->scenes[0];
	scene->SetAmbientLightIntensity(glm::vec3(0.3f, 0.3f, 0.3f));

	camera = scene->GetRootNode()->GetComponentInChildren<lavos::CameraComponent>();

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
}

void MainWindowRenderer::initSwapChainResources()
{
	if(color_render_target != nullptr)
	{
		color_render_target->SwapchainChanged();
		return;
	}

	color_render_target = new QVulkanWindowColorRenderTarget(window);
	depth_render_target = new QVulkanWindowDepthRenderTarget(window);

	renderer = new lavos::Renderer(engine, color_render_target, depth_render_target);
	renderer->AddMaterial(material);

	renderer->SetScene(scene);
	renderer->SetCamera(camera);
}

void MainWindowRenderer::releaseSwapChainResources()
{
}

void MainWindowRenderer::releaseResources()
{
	delete renderer;
	delete depth_render_target;
	delete color_render_target;
	delete asset_container;
	delete material;
	delete engine;
}

void MainWindowRenderer::startNextFrame()
{
	renderer->DrawFrame(static_cast<uint32_t>(window->currentSwapChainImageIndex()), {}, {}, {});
	window->frameReady();
}
