
#if defined(__ANDROID__)
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <common.h>
#endif

#include <iostream>
#include <set>
#include <fstream>
#include <chrono>

#include <glm_config.h>
#include <glm/gtc/matrix_transform.hpp>

#include "mesh_application.h"

#include <vulkan/vulkan.h>
#include <engine.h>
#include <material.h>
#include <gltf_loader.h>

#include <shader_load.h>
#include "../../thirdparty/stb_image.h"


void MeshApplication::InitVulkan()
{
	DemoApplication::InitVulkan();

	CreateDepthResources();
	CreateRenderPasses();

	material = new engine::Material(engine);
	renderer = new engine::Renderer(engine, swapchain_extent, render_pass);
	renderer->AddMaterial(material);

	CreateFramebuffers();
	CreateCommandPool();

	auto gltf = new engine::GLTF(renderer, "data/gltftest.gltf");
	mesh = gltf->GetMeshes().front();
	gltf->GetMeshes().clear();

	material_instance = gltf->GetMaterialInstances().front();
	gltf->GetMaterialInstances().clear();

	CreateVertexBuffer();
	CreateIndexBuffer();

	CreateCommandBuffers();
}

void MeshApplication::CreateDepthResources()
{
	depth_format = engine->FindDepthFormat();

	depth_image = engine->Create2DImage(swapchain_extent.width, swapchain_extent.height, depth_format,
										vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
										VMA_MEMORY_USAGE_GPU_ONLY);

	depth_image_view = engine->GetVkDevice().createImageView(vk::ImageViewCreateInfo()
		.setImage(depth_image.image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(depth_format)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1)));

	engine->TransitionImageLayout(depth_image.image, depth_format, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

void MeshApplication::CreateRenderPasses()
{
	 auto color_attachment = vk::AttachmentDescription()
		.setFormat(swapchain_image_format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
	vk::AttachmentReference color_attachment_ref(0, vk::ImageLayout::eColorAttachmentOptimal);

	auto depth_attachment = vk::AttachmentDescription()
		.setFormat(depth_format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
	vk::AttachmentReference depth_attachment_ref(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);



	auto subpass = vk::SubpassDescription()
		.setColorAttachmentCount(1)
		.setPColorAttachments(&color_attachment_ref)
		.setPDepthStencilAttachment(&depth_attachment_ref);

	auto subpass_dependency = vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			//.setSrcAccessMask(0)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);


	std::array<vk::AttachmentDescription, 2> attachments = { color_attachment, depth_attachment };

	render_pass = engine->GetVkDevice().createRenderPass(
			vk::RenderPassCreateInfo()
				.setAttachmentCount(attachments.size())
				.setPAttachments(attachments.data())
				.setSubpassCount(1)
				.setPSubpasses(&subpass)
				.setDependencyCount(1)
				.setPDependencies(&subpass_dependency));
}


void MeshApplication::CreateVertexBuffer()
{
	vk::DeviceSize size = sizeof(mesh->vertices[0]) * mesh->vertices.size();

	auto staging_buffer = engine->CreateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY);
	void *data = engine->MapMemory(staging_buffer.allocation);
	memcpy(data, mesh->vertices.data(), size);
	engine->UnmapMemory(staging_buffer.allocation);

	vertex_buffer = engine->CreateBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
										 VMA_MEMORY_USAGE_GPU_ONLY);

	engine->CopyBuffer(staging_buffer.buffer, vertex_buffer.buffer, size);

	engine->DestroyBuffer(staging_buffer);
}

void MeshApplication::CreateIndexBuffer()
{
	vk::DeviceSize size = sizeof(mesh->indices[0]) * mesh->indices.size();

	auto staging_buffer = engine->CreateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY);
	void *data = engine->MapMemory(staging_buffer.allocation);
	memcpy(data, mesh->indices.data(), size);
	engine->UnmapMemory(staging_buffer.allocation);

	index_buffer = engine->CreateBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
										VMA_MEMORY_USAGE_GPU_ONLY);

	engine->CopyBuffer(staging_buffer.buffer, index_buffer.buffer, size);

	engine->DestroyBuffer(staging_buffer);
}


void MeshApplication::CreateFramebuffers()
{
	swapchain_framebuffers.resize(swapchain_image_views.size());

	for(size_t i=0; i<swapchain_image_views.size(); i++)
	{
		std::array<vk::ImageView, 2> attachments = {
			swapchain_image_views[i],
			depth_image_view
		};

		auto framebuffer_info = vk::FramebufferCreateInfo()
			.setRenderPass(render_pass)
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
					.setRenderPass(render_pass)
					.setFramebuffer(swapchain_framebuffers[i])
					.setRenderArea(vk::Rect2D({ 0, 0 }, swapchain_extent))
					.setClearValueCount(clear_values.size())
					.setPClearValues(clear_values.data()),
				vk::SubpassContents::eInline);


		command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline);
		command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.pipeline_layout, 0, renderer->GetDescriptorSet(), nullptr);
		command_buffer.bindVertexBuffers(0, { vertex_buffer.buffer }, { 0 });
		command_buffer.bindIndexBuffer(index_buffer.buffer, 0, vk::IndexType::eUint16);

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

	CreateRenderPasses();
	//CreatePipeline();
	CreateDepthResources();
	CreateFramebuffers();
	CreateCommandBuffers();
}

void MeshApplication::CleanupSwapchain()
{
	auto device = engine->GetVkDevice();

	device.freeCommandBuffers(command_pool, command_buffers);

	device.destroyImageView(depth_image_view);
	engine->DestroyImage(depth_image);

	device.destroyRenderPass(render_pass);

	DemoApplication::CleanupSwapchain();
}

void MeshApplication::CleanupApplication()
{
	auto device = engine->GetVkDevice();


	device.destroyCommandPool(command_pool);

	delete material_instance;

	engine->DestroyBuffer(index_buffer);
	engine->DestroyBuffer(vertex_buffer);
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
