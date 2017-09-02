
#if defined(__ANDROID__)
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <common.h>
#endif

#include <iostream>
#include <set>
#include <fstream>

#include "triangle_application.h"

#include <vulkan/vulkan.h>
#include <engine.h>

#include <shader_load.h>



void TriangleApplication::InitVulkan()
{
	DemoApplication::InitVulkan();

	CreateRenderPasses();
	CreatePipeline();
	CreateFramebuffers();
	CreateCommandPool();
	CreateCommandBuffers();
}

void TriangleApplication::CreateRenderPasses()
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

	auto subpass = vk::SubpassDescription()
		.setColorAttachmentCount(1)
		.setPColorAttachments(&color_attachment_ref);

	auto subpass_dependency = vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			//.setSrcAccessMask(0)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	render_pass = engine->GetVkDevice().createRenderPass(
			vk::RenderPassCreateInfo()
				.setAttachmentCount(1)
				.setPAttachments(&color_attachment)
				.setSubpassCount(1)
				.setPSubpasses(&subpass)
				.setDependencyCount(1)
				.setPDependencies(&subpass_dependency));
}

vk::ShaderModule TriangleApplication::CreateShaderModule(const std::vector<char> &code)
{
	return engine->GetVkDevice().createShaderModule(
			vk::ShaderModuleCreateInfo()
					.setCodeSize(code.size())
					.setPCode(reinterpret_cast<const uint32_t *>(code.data())));
}

void TriangleApplication::CreatePipeline()
{
	auto vert_shader_module = CreateShaderModule(ReadSPIRVShader("triangle.vert"));
	auto frag_shader_module = CreateShaderModule(ReadSPIRVShader("triangle.frag"));

	vk::PipelineShaderStageCreateInfo shader_stages[] = {
			vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(),
											  vk::ShaderStageFlagBits::eVertex,
											  vert_shader_module,
											  "main"),
			vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(),
											  vk::ShaderStageFlagBits::eFragment,
											  frag_shader_module,
											  "main")
	};

	auto vertex_input_info = vk::PipelineVertexInputStateCreateInfo();

	auto input_assembly_info = vk::PipelineInputAssemblyStateCreateInfo()
		.setTopology(vk::PrimitiveTopology::eTriangleList);

	vk::Viewport viewport(0.0f, 0.0f, swapchain_extent.width, swapchain_extent.height, 0.0f, 1.0f);
	vk::Rect2D scissor({0, 0}, swapchain_extent);

	auto viewport_state_info = vk::PipelineViewportStateCreateInfo()
		.setViewportCount(1)
		.setPViewports(&viewport)
		.setScissorCount(1)
		.setPScissors(&scissor);

	auto rasterizer_info = vk::PipelineRasterizationStateCreateInfo()
		.setDepthClampEnable(VK_FALSE)
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1.0f)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eClockwise)
		.setDepthBiasEnable(VK_FALSE);


	auto multisample_info = vk::PipelineMultisampleStateCreateInfo()
		.setSampleShadingEnable(VK_FALSE)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);


	auto color_blend_attachment = vk::PipelineColorBlendAttachmentState()
		.setColorWriteMask(vk::ColorComponentFlagBits::eR
						   | vk::ColorComponentFlagBits::eG
						   | vk::ColorComponentFlagBits::eB
						   | vk::ColorComponentFlagBits::eA)
		.setBlendEnable(VK_FALSE);

	auto color_blend_info = vk::PipelineColorBlendStateCreateInfo()
		.setLogicOpEnable(VK_FALSE)
		.setAttachmentCount(1)
		.setPAttachments(&color_blend_attachment);


	auto pipeline_layout_info = vk::PipelineLayoutCreateInfo();
	pipeline_layout = engine->GetVkDevice().createPipelineLayout(pipeline_layout_info);


	auto pipeline_info = vk::GraphicsPipelineCreateInfo()
		.setStageCount(2)
		.setPStages(shader_stages)
		.setPVertexInputState(&vertex_input_info)
		.setPInputAssemblyState(&input_assembly_info)
		.setPViewportState(&viewport_state_info)
		.setPRasterizationState(&rasterizer_info)
		.setPMultisampleState(&multisample_info)
		.setPDepthStencilState(nullptr)
		.setPColorBlendState(&color_blend_info)
		.setPDynamicState(nullptr)
		.setLayout(pipeline_layout)
		.setRenderPass(render_pass)
		.setSubpass(0);


	pipeline = engine->GetVkDevice().createGraphicsPipeline(vk::PipelineCache() /*nullptr*/, pipeline_info);


	engine->GetVkDevice().destroyShaderModule(vert_shader_module);
	engine->GetVkDevice().destroyShaderModule(frag_shader_module);
}

void TriangleApplication::CreateFramebuffers()
{
	swapchain_framebuffers.resize(swapchain_image_views.size());

	for(size_t i=0; i<swapchain_image_views.size(); i++)
	{
		vk::ImageView attachments[] = {
			swapchain_image_views[i]
		};

		auto framebuffer_info = vk::FramebufferCreateInfo()
			.setRenderPass(render_pass)
			.setAttachmentCount(1)
			.setPAttachments(attachments)
			.setWidth(swapchain_extent.width)
			.setHeight(swapchain_extent.height)
			.setLayers(1);

		swapchain_framebuffers[i] = engine->GetVkDevice().createFramebuffer(framebuffer_info);
	}
}


void TriangleApplication::CreateCommandPool()
{
	auto queue_family_indices = engine->GetQueueFamilyIndices();

	auto command_pool_info = vk::CommandPoolCreateInfo()
		.setQueueFamilyIndex(static_cast<uint32_t>(queue_family_indices.graphics_family));

	command_pool = engine->GetVkDevice().createCommandPool(command_pool_info);
}

void TriangleApplication::CreateCommandBuffers()
{
	command_buffers = engine->GetVkDevice().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
					.setCommandPool(command_pool)
					.setLevel(vk::CommandBufferLevel::ePrimary)
					.setCommandBufferCount(static_cast<uint32_t>(swapchain_image_views.size())));

	for(size_t i=0; i<command_buffers.size(); i++)
	{
		const auto &command_buffer = command_buffers[i];

		command_buffer.begin({ vk::CommandBufferUsageFlagBits::eSimultaneousUse });

		vk::ClearValue clear_color_value = vk::ClearColorValue(std::array<float, 4>{{ 0.0f, 0.0f, 0.0f, 1.0f }});

		command_buffer.beginRenderPass(
				vk::RenderPassBeginInfo()
					.setRenderPass(render_pass)
					.setFramebuffer(swapchain_framebuffers[i])
					.setRenderArea(vk::Rect2D({ 0, 0 }, swapchain_extent))
					.setClearValueCount(1)
					.setPClearValues(&clear_color_value),
				vk::SubpassContents::eInline);

		command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
		command_buffer.draw(3, 1, 0, 0);

		command_buffer.endRenderPass();

		command_buffer.end();
	}
}


void TriangleApplication::DrawFrame(uint32_t image_index)
{
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

void TriangleApplication::RecreateSwapchain()
{
	DemoApplication::RecreateSwapchain();

	CreateRenderPasses();
	CreatePipeline();
	CreateFramebuffers();
	CreateCommandBuffers();
}

void TriangleApplication::CleanupSwapchain()
{
	auto device = engine->GetVkDevice();

	device.freeCommandBuffers(command_pool, command_buffers);

	device.destroyPipeline(pipeline);
	device.destroyPipelineLayout(pipeline_layout);
	device.destroyRenderPass(render_pass);
}

void TriangleApplication::Cleanup()
{
	auto device = engine->GetVkDevice();

	device.destroyCommandPool(command_pool);
}