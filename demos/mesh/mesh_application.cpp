
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
#include <material.h>
#include <gltf_loader.h>


void MeshApplication::InitVulkan()
{
	DemoApplication::InitVulkan();

	material = new engine::Material(engine);
	renderer = new engine::Renderer(engine, swapchain_extent, swapchain_image_format);
	renderer->AddMaterial(material);

	CreateFramebuffers();
	CreateCommandPool();

	auto gltf = new engine::GLTF(renderer, "data/gltftest.gltf");
	mesh = gltf->GetMeshes().front();
	gltf->GetMeshes().clear();

	material_instance = gltf->GetMaterialInstances().front();
	gltf->GetMaterialInstances().clear();

	CreateCommandBuffers();
}


void MeshApplication::CreateFramebuffers()
{
	swapchain_framebuffers.resize(swapchain_image_views.size());

	for(size_t i=0; i<swapchain_image_views.size(); i++)
	{
		std::array<vk::ImageView, 2> attachments = {
			swapchain_image_views[i],
			renderer->GetDepthImageView()
		};

		auto framebuffer_info = vk::FramebufferCreateInfo()
			.setRenderPass(renderer->GetRenderPass())
			.setAttachmentCount(attachments.size())
			.setPAttachments(attachments.data())
			.setWidth(swapchain_extent.width)
			.setHeight(swapchain_extent.height)
			.setLayers(1);

		swapchain_framebuffers[i] = engine->GetVkDevice().createFramebuffer(framebuffer_info);
	}
}

void MeshApplication::CreateCommandPool()
{
	auto queue_family_indices = engine->GetQueueFamilyIndices();

	auto command_pool_info = vk::CommandPoolCreateInfo()
		.setQueueFamilyIndex(static_cast<uint32_t>(queue_family_indices.graphics_family));

	command_pool = engine->GetVkDevice().createCommandPool(command_pool_info);
}

void MeshApplication::CreateCommandBuffers()
{
	auto pipeline = renderer->GetMaterialPipeline(0);

	command_buffers = engine->GetVkDevice().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
					.setCommandPool(command_pool)
					.setLevel(vk::CommandBufferLevel::ePrimary)
					.setCommandBufferCount(static_cast<uint32_t>(swapchain_image_views.size())));

	for(size_t i=0; i<command_buffers.size(); i++)
	{
		const auto &command_buffer = command_buffers[i];

		command_buffer.begin({ vk::CommandBufferUsageFlagBits::eSimultaneousUse });

		std::array<vk::ClearValue, 2> clear_values = {
			vk::ClearColorValue(std::array<float, 4>{{ 0.0f, 0.0f, 0.0f, 1.0f }}),
			vk::ClearDepthStencilValue(1.0f, 0)
		};

		command_buffer.beginRenderPass(
				vk::RenderPassBeginInfo()
					.setRenderPass(renderer->GetRenderPass())
					.setFramebuffer(swapchain_framebuffers[i])
					.setRenderArea(vk::Rect2D({ 0, 0 }, swapchain_extent))
					.setClearValueCount(clear_values.size())
					.setPClearValues(clear_values.data()),
				vk::SubpassContents::eInline);


		command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline);
		command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.pipeline_layout, 0, renderer->GetDescriptorSet(), nullptr);
		command_buffer.bindVertexBuffers(0, { mesh->vertex_buffer.buffer }, { 0 });
		command_buffer.bindIndexBuffer(mesh->index_buffer.buffer, 0, vk::IndexType::eUint16);

		for(auto primitive : mesh->primitives)
		{
			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.pipeline_layout, 1, primitive.material_instance->GetDescriptorSet(), nullptr);
			command_buffer.drawIndexed(primitive.indices_count, 1, primitive.indices_offset, 0, 0);
		}

		command_buffer.endRenderPass();

		command_buffer.end();
	}
}


void MeshApplication::DrawFrame(uint32_t image_index)
{
	renderer->UpdateMatrixUniformBuffer();

	vk::Semaphore wait_semaphores[] = { image_available_semaphore };
	vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	vk::Semaphore signal_semaphores[] = { render_finished_semaphore };

	engine->GetGraphicsQueue().submit(
			vk::SubmitInfo()
				  .setWaitSemaphoreCount(1)
				  .setPWaitSemaphores(wait_semaphores)
				  .setPWaitDstStageMask(wait_stages)
				  .setCommandBufferCount(1)
				  .setPCommandBuffers(&command_buffers[image_index])
				  .setSignalSemaphoreCount(1)
				  .setPSignalSemaphores(signal_semaphores),
			vk::Fence() /*nullptr*/);
}

void MeshApplication::RecreateSwapchain()
{
	DemoApplication::RecreateSwapchain();

	renderer->ResizeScreen(swapchain_extent);

	CreateFramebuffers();
	CreateCommandBuffers();
}

void MeshApplication::CleanupSwapchain()
{
	auto device = engine->GetVkDevice();

	device.freeCommandBuffers(command_pool, command_buffers);

	DemoApplication::CleanupSwapchain();
}

void MeshApplication::CleanupApplication()
{
	auto device = engine->GetVkDevice();

	device.destroyCommandPool(command_pool);

	delete material_instance;
	delete mesh;
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
