
#if defined(__ANDROID__)
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <common.h>
#endif

#include <iostream>
#include <set>
#include <fstream>
#include <chrono>

#include <glm_config.h>

#include "mesh_application.h"

#include <vulkan/vulkan.h>
#include <engine.h>
#include <asset_container.h>
#include <mesh_component.h>


void MeshApplication::InitVulkan()
{
	DemoApplication::InitVulkan();

	material = new engine::Material(engine);
	renderer = new engine::Renderer(engine, swapchain_extent, swapchain_image_format, swapchain_image_views);
	renderer->AddMaterial(material);

	asset_container = engine::AssetContainer::LoadFromGLTF(engine, material, "data/gltftest.gltf");

	renderer->SetScene(asset_container->scenes[0]);

	material_instance = asset_container->material_instances.front();
}

void MeshApplication::DrawFrame(uint32_t image_index)
{
	renderer->DrawFrame(image_index,
						{ image_available_semaphore },
						{ vk::PipelineStageFlagBits::eColorAttachmentOutput },
						{ render_finished_semaphore });
}

void MeshApplication::RecreateSwapchain()
{
	DemoApplication::RecreateSwapchain();
	renderer->ResizeScreen(swapchain_extent, swapchain_image_views);
}

void MeshApplication::CleanupApplication()
{
	delete asset_container;
	delete renderer;
}


#ifndef __ANDROID__
int main()
{
	MeshApplication app;

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
