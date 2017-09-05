
#if defined(__ANDROID__)
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <common.h>
#endif

#include <iostream>
#include <set>
#include <fstream>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "depthbuffer_application.h"

#include <vulkan/vulkan.h>
#include <engine.h>

#include <shader_load.h>
#include "../../thirdparty/stb_image.h"


void DepthBufferApplication::InitVulkan()
{
	DemoApplication::InitVulkan();

	CreateDepthResources();
	CreateRenderPasses();
	CreateDescriptorSetLayout();
	CreatePipeline();
	CreateFramebuffers();
	CreateCommandPool();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateTextureImage();
	CreateTextureImageView();
	CreateTextureSampler();
	CreateMatrixUniformBuffer();
	CreateDescriptorPool();
	CreateDescriptorSet();
	CreateCommandBuffers();
}

void DepthBufferApplication::CreateDepthResources()
{
	depth_format = engine->FindDepthFormat();

	depth_image = engine->Create2DImageWithMemory(swapchain_extent.width, swapchain_extent.height, depth_format,
												  vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
												  vk::MemoryPropertyFlagBits::eDeviceLocal, &depth_image_memory);

	depth_image_view = engine->GetVkDevice().createImageView(vk::ImageViewCreateInfo()
		.setImage(depth_image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(depth_format)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1)));

	engine->TransitionImageLayout(depth_image, depth_format, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

void DepthBufferApplication::CreateRenderPasses()
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

void DepthBufferApplication::CreateDescriptorSetLayout()
{
	std::vector<vk::DescriptorSetLayoutBinding> bindings = {
		vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex),

		vk::DescriptorSetLayoutBinding()
			.setBinding(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1)
			.setPImmutableSamplers(nullptr)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment)
	};

	auto create_info = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(static_cast<uint32_t>(bindings.size()))
		.setPBindings(bindings.data());

	descriptor_set_layout = engine->GetVkDevice().createDescriptorSetLayout(create_info);
}


vk::ShaderModule DepthBufferApplication::CreateShaderModule(const std::vector<char> &code)
{
	return engine->GetVkDevice().createShaderModule(
			vk::ShaderModuleCreateInfo()
					.setCodeSize(code.size())
					.setPCode(reinterpret_cast<const uint32_t *>(code.data())));
}


void DepthBufferApplication::CreatePipeline()
{
	auto vert_shader_module = CreateShaderModule(ReadSPIRVShader("texture.vert"));
	auto frag_shader_module = CreateShaderModule(ReadSPIRVShader("texture.frag"));

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

	auto vertex_binding_description = Vertex::GetBindingDescription();
	auto vertex_attribute_description = Vertex::GetAttributeDescription();

	auto vertex_input_info = vk::PipelineVertexInputStateCreateInfo()
		.setVertexBindingDescriptionCount(1)
		.setPVertexBindingDescriptions(&vertex_binding_description)
		.setVertexAttributeDescriptionCount(vertex_attribute_description.size())
		.setPVertexAttributeDescriptions(vertex_attribute_description.data());

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
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setDepthBiasEnable(VK_FALSE);


	auto multisample_info = vk::PipelineMultisampleStateCreateInfo()
		.setSampleShadingEnable(VK_FALSE)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);


	auto depth_stencil_info = vk::PipelineDepthStencilStateCreateInfo()
		.setDepthTestEnable(VK_TRUE)
		.setDepthWriteEnable(VK_TRUE)
		.setDepthCompareOp(vk::CompareOp::eLess)
		.setDepthBoundsTestEnable(VK_FALSE)
		.setStencilTestEnable(VK_FALSE);


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


	auto pipeline_layout_info = vk::PipelineLayoutCreateInfo()
		.setSetLayoutCount(1)
		.setPSetLayouts(&descriptor_set_layout);

	pipeline_layout = engine->GetVkDevice().createPipelineLayout(pipeline_layout_info);


	auto pipeline_info = vk::GraphicsPipelineCreateInfo()
		.setStageCount(2)
		.setPStages(shader_stages)
		.setPVertexInputState(&vertex_input_info)
		.setPInputAssemblyState(&input_assembly_info)
		.setPViewportState(&viewport_state_info)
		.setPRasterizationState(&rasterizer_info)
		.setPMultisampleState(&multisample_info)
		.setPDepthStencilState(&depth_stencil_info)
		.setPColorBlendState(&color_blend_info)
		.setPDynamicState(nullptr)
		.setLayout(pipeline_layout)
		.setRenderPass(render_pass)
		.setSubpass(0);


	pipeline = engine->GetVkDevice().createGraphicsPipeline(vk::PipelineCache() /*nullptr*/, pipeline_info);


	engine->GetVkDevice().destroyShaderModule(vert_shader_module);
	engine->GetVkDevice().destroyShaderModule(frag_shader_module);
}


void DepthBufferApplication::CreateVertexBuffer()
{
	auto device = engine->GetVkDevice();

	vk::DeviceSize size = sizeof(vertices[0]) * vertices.size();


	vk::DeviceMemory staging_buffer_memory;
	auto staging_buffer = engine->CreateBufferWithMemory(size, vk::BufferUsageFlagBits::eTransferSrc,
														 vk::MemoryPropertyFlagBits::eHostVisible |
														 vk::MemoryPropertyFlagBits::eHostCoherent,
														 &staging_buffer_memory);

	void *data = device.mapMemory(staging_buffer_memory, 0, size);
	memcpy(data, vertices.data(), size);
	device.unmapMemory(staging_buffer_memory);


	vertex_buffer = engine->CreateBufferWithMemory(size, vk::BufferUsageFlagBits::eTransferDst |
														 vk::BufferUsageFlagBits::eVertexBuffer,
												   vk::MemoryPropertyFlagBits::eDeviceLocal,
												   &vertex_buffer_memory);

	engine->CopyBuffer(staging_buffer, vertex_buffer, size);

	device.destroyBuffer(staging_buffer);
	device.freeMemory(staging_buffer_memory);
}

void DepthBufferApplication::CreateIndexBuffer()
{
	auto device = engine->GetVkDevice();

	vk::DeviceSize size = sizeof(indices[0]) * indices.size();

	vk::DeviceMemory staging_buffer_memory;
	auto staging_buffer = engine->CreateBufferWithMemory(size, vk::BufferUsageFlagBits::eTransferSrc,
														 vk::MemoryPropertyFlagBits::eHostVisible |
														 vk::MemoryPropertyFlagBits::eHostCoherent,
														 &staging_buffer_memory);

	void *data = device.mapMemory(staging_buffer_memory, 0, size);
	memcpy(data, indices.data(), size);
	device.unmapMemory(staging_buffer_memory);


	index_buffer = engine->CreateBufferWithMemory(size, vk::BufferUsageFlagBits::eTransferDst |
														vk::BufferUsageFlagBits::eIndexBuffer,
												  vk::MemoryPropertyFlagBits::eDeviceLocal,
												  &index_buffer_memory);

	engine->CopyBuffer(staging_buffer, index_buffer, size);

	device.destroyBuffer(staging_buffer);
	device.freeMemory(staging_buffer_memory);
}

void DepthBufferApplication::CreateMatrixUniformBuffer()
{
	vk::DeviceSize size = sizeof(MatrixUniformBuffer);
	matrix_uniform_buffer = engine->CreateBufferWithMemory(size, vk::BufferUsageFlagBits::eUniformBuffer,
														   vk::MemoryPropertyFlagBits::eHostVisible |
														   vk::MemoryPropertyFlagBits::eHostCoherent,
														   &matrix_uniform_buffer_memory);


}

void DepthBufferApplication::CreateFramebuffers()
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

void DepthBufferApplication::CreateCommandPool()
{
	auto queue_family_indices = engine->GetQueueFamilyIndices();

	auto command_pool_info = vk::CommandPoolCreateInfo()
		.setQueueFamilyIndex(static_cast<uint32_t>(queue_family_indices.graphics_family));

	command_pool = engine->GetVkDevice().createCommandPool(command_pool_info);
}

void DepthBufferApplication::CreateCommandBuffers()
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

		command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
		command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout, 0, descriptor_set, nullptr);
		command_buffer.bindVertexBuffers(0, { vertex_buffer }, { 0 });
		command_buffer.bindIndexBuffer(index_buffer, 0, vk::IndexType::eUint16);
		command_buffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		command_buffer.endRenderPass();

		command_buffer.end();
	}
}

void DepthBufferApplication::CreateDescriptorPool()
{
	std::vector<vk::DescriptorPoolSize> pool_sizes = {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1),
	};

	auto create_info = vk::DescriptorPoolCreateInfo()
		.setPoolSizeCount(static_cast<uint32_t>(pool_sizes.size()))
		.setPPoolSizes(pool_sizes.data())
		.setMaxSets(1);

	descriptor_pool = engine->GetVkDevice().createDescriptorPool(create_info);
}

void DepthBufferApplication::CreateDescriptorSet()
{
	vk::DescriptorSetLayout layouts[] = { descriptor_set_layout };

	auto alloc_info = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(descriptor_pool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(layouts);

	descriptor_set = *engine->GetVkDevice().allocateDescriptorSets(alloc_info).begin();

	auto buffer_info = vk::DescriptorBufferInfo()
		.setBuffer(matrix_uniform_buffer)
		.setOffset(0)
		.setRange(sizeof(MatrixUniformBuffer));

	auto buffer_write = vk::WriteDescriptorSet()
		.setDstSet(descriptor_set)
		.setDstBinding(0)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setPBufferInfo(&buffer_info);


	auto image_info = vk::DescriptorImageInfo()
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setImageView(texture_image_view)
		.setSampler(texture_sampler);

	auto image_write = vk::WriteDescriptorSet()
		.setDstSet(descriptor_set)
		.setDstBinding(1)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1)
		.setPImageInfo(&image_info);

	engine->GetVkDevice().updateDescriptorSets({buffer_write, image_write}, nullptr);
}

void DepthBufferApplication::CreateTextureImage()
{
	int width, height, channels;
	stbi_uc *pixels = stbi_load("data/tex.jpg", &width, &height, &channels, STBI_rgb_alpha);

	vk::DeviceSize image_size = static_cast<vk::DeviceSize>(width * height * 4);

	if (!pixels)
		throw std::runtime_error("failed to load texture image!");


	auto device = engine->GetVkDevice();

	vk::DeviceMemory staging_buffer_memory;
	vk::Buffer staging_buffer = engine->CreateBufferWithMemory(image_size, vk::BufferUsageFlagBits::eTransferSrc,
															   vk::MemoryPropertyFlagBits::eHostVisible |
															   vk::MemoryPropertyFlagBits::eHostCoherent,
															   &staging_buffer_memory);

	void *data = device.mapMemory(staging_buffer_memory, 0, image_size);
	memcpy(data, pixels, image_size);
	device.unmapMemory(staging_buffer_memory);

	stbi_image_free(pixels);


	vk::Format format = vk::Format::eR8G8B8A8Unorm;

	texture_image = engine->Create2DImageWithMemory(width, height, format, vk::ImageTiling::eOptimal,
													vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
													vk::MemoryPropertyFlagBits::eDeviceLocal, &texture_image_memory);


	engine->TransitionImageLayout(texture_image, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	engine->CopyBufferTo2DImage(staging_buffer, texture_image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	engine->TransitionImageLayout(texture_image, format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);


	device.destroyBuffer(staging_buffer);
	device.freeMemory(staging_buffer_memory);
}

void DepthBufferApplication::CreateTextureImageView()
{
	auto create_info = vk::ImageViewCreateInfo()
		.setImage(texture_image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(vk::Format::eR8G8B8A8Unorm)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

	texture_image_view = engine->GetVkDevice().createImageView(create_info);
}

void DepthBufferApplication::CreateTextureSampler()
{
	auto create_info = vk::SamplerCreateInfo()
		.setMagFilter(vk::Filter::eLinear)
		.setMinFilter(vk::Filter::eLinear)
		.setAddressModeU(vk::SamplerAddressMode::eRepeat)
		.setAddressModeV(vk::SamplerAddressMode::eRepeat)
		.setAddressModeW(vk::SamplerAddressMode::eRepeat)
		.setAnisotropyEnable(VK_TRUE)
		.setMaxAnisotropy(16)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(vk::CompareOp::eAlways)
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setMipLodBias(0.0f)
		.setMinLod(0.0f)
		.setMaxLod(0.0f);

	texture_sampler = engine->GetVkDevice().createSampler(create_info);
}

void DepthBufferApplication::UpdateMatrixUniformBuffer()
{
	static auto start_time = std::chrono::high_resolution_clock::now();
	auto current_time = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time).count() / 1e6f;

	MatrixUniformBuffer matrix_ubo;
	matrix_ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	matrix_ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	matrix_ubo.projection = glm::perspective(glm::radians(45.0f), (float)swapchain_extent.width / (float)swapchain_extent.height, 0.1f, 10.0f);
	matrix_ubo.projection[1][1] *= -1.0f;

	void *data = engine->GetVkDevice().mapMemory(matrix_uniform_buffer_memory, 0, sizeof(matrix_ubo));
	memcpy(data, &matrix_ubo, sizeof(matrix_ubo));
	engine->GetVkDevice().unmapMemory(matrix_uniform_buffer_memory);
}

void DepthBufferApplication::DrawFrame(uint32_t image_index)
{
	UpdateMatrixUniformBuffer();

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

void DepthBufferApplication::RecreateSwapchain()
{
	DemoApplication::RecreateSwapchain();

	CreateRenderPasses();
	CreatePipeline();
	CreateDepthResources();
	CreateFramebuffers();
	CreateCommandBuffers();
}

void DepthBufferApplication::CleanupSwapchain()
{
	auto device = engine->GetVkDevice();

	device.freeCommandBuffers(command_pool, command_buffers);

	device.destroyImageView(depth_image_view);
	device.destroyImage(depth_image);
	device.freeMemory(depth_image_memory);

	device.destroyPipeline(pipeline);
	device.destroyPipelineLayout(pipeline_layout);

	device.destroyRenderPass(render_pass);

	DemoApplication::CleanupSwapchain();
}

void DepthBufferApplication::CleanupApplication()
{
	auto device = engine->GetVkDevice();

	device.destroyDescriptorPool(descriptor_pool);

	device.destroyDescriptorSetLayout(descriptor_set_layout);

	device.destroyCommandPool(command_pool);

	device.destroySampler(texture_sampler);
	device.destroyImageView(texture_image_view);
	device.destroyImage(texture_image);
	device.freeMemory(texture_image_memory);

	device.destroyBuffer(index_buffer);
	device.freeMemory(index_buffer_memory);

	device.destroyBuffer(vertex_buffer);
	device.freeMemory(vertex_buffer_memory);

	device.destroyBuffer(matrix_uniform_buffer);
	device.freeMemory(matrix_uniform_buffer_memory);
}


#ifndef __ANDROID__
int main()
{
	DepthBufferApplication app;

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
