
#include <chrono>
#include "glm_config.h"
#include <glm/gtc/matrix_transform.hpp>

#include "renderer.h"
#include "shader_load.h"
#include "vertex.h"

using namespace engine;

Renderer::Renderer(Engine *engine, vk::Extent2D screen_extent, vk::Format format, std::vector<vk::ImageView> dst_image_views)
	: engine(engine), dst_image_views(dst_image_views)
{
	this->screen_extent = screen_extent;
	this->format = format;

	CreateDescriptorPool();
	CreateDescriptorSetLayout();
	CreateMatrixUniformBuffer();
	CreateDescriptorSet();

	CreateDepthResources();
	CreateRenderPasses();

	CreateFramebuffers();

	CreateCommandPool();
}

Renderer::~Renderer()
{
	auto &device = engine->GetVkDevice();

	device.destroyDescriptorSetLayout(descriptor_set_layout);

	for(auto &material_pipeline : material_pipelines)
		DestroyMaterialPipeline(material_pipeline);

	device.destroyDescriptorPool(descriptor_pool);

	engine->DestroyBuffer(matrix_uniform_buffer);

	CleanupFramebuffers();
	CleanupCommandPool();

	CleanupDepthResources();
	CleanupRenderPasses();
}

void Renderer::CreateCommandPool()
{
	auto queue_family_indices = engine->GetQueueFamilyIndices();

	auto command_pool_info = vk::CommandPoolCreateInfo()
		.setQueueFamilyIndex(static_cast<uint32_t>(queue_family_indices.graphics_family));

	command_pool = engine->GetVkDevice().createCommandPool(command_pool_info);
}

void Renderer::CleanupCommandPool()
{
	engine->GetVkDevice().destroyCommandPool(command_pool);
}

void Renderer::CreateFramebuffers()
{
	dst_framebuffers.resize(dst_image_views.size());

	for(size_t i=0; i<dst_image_views.size(); i++)
	{
		std::array<vk::ImageView, 2> attachments = {
			dst_image_views[i],
			depth_image_view
		};

		auto framebuffer_info = vk::FramebufferCreateInfo()
			.setRenderPass(render_pass)
			.setAttachmentCount(attachments.size())
			.setPAttachments(attachments.data())
			.setWidth(screen_extent.width)
			.setHeight(screen_extent.height)
			.setLayers(1);

		dst_framebuffers[i] = engine->GetVkDevice().createFramebuffer(framebuffer_info);
	}
}

void Renderer::CleanupFramebuffers()
{
	for(vk::Framebuffer framebuffer : dst_framebuffers)
		engine->GetVkDevice().destroyFramebuffer(framebuffer);
}

void Renderer::CreateDescriptorPool()
{
	std::vector<vk::DescriptorPoolSize> pool_sizes = {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 2),
	};

	auto create_info = vk::DescriptorPoolCreateInfo()
		.setPoolSizeCount(static_cast<uint32_t>(pool_sizes.size()))
		.setPPoolSizes(pool_sizes.data())
		.setMaxSets(3);

	descriptor_pool = engine->GetVkDevice().createDescriptorPool(create_info);
}

void Renderer::CreateDescriptorSetLayout()
{
	std::vector<vk::DescriptorSetLayoutBinding> bindings = {
		vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex),
	};

	auto create_info = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(static_cast<uint32_t>(bindings.size()))
		.setPBindings(bindings.data());

	descriptor_set_layout = engine->GetVkDevice().createDescriptorSetLayout(create_info);
}


void Renderer::CreateMatrixUniformBuffer()
{
	matrix_uniform_buffer = engine->CreateBuffer(sizeof(MatrixUniformBuffer),
												 vk::BufferUsageFlagBits::eUniformBuffer,
												 VMA_MEMORY_USAGE_CPU_ONLY);
}

void Renderer::UpdateMatrixUniformBuffer()
{
	static auto start_time = std::chrono::high_resolution_clock::now();
	auto current_time = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time).count() / 1e6f;

	MatrixUniformBuffer matrix_ubo;
	matrix_ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	matrix_ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	matrix_ubo.projection = glm::perspective(glm::radians(60.0f), (float)screen_extent.width / (float)screen_extent.height, 0.1f, 10.0f);
	matrix_ubo.projection[1][1] *= -1.0f;

	void *data = engine->MapMemory(matrix_uniform_buffer.allocation);
	memcpy(data, &matrix_ubo, sizeof(matrix_ubo));
	engine->UnmapMemory(matrix_uniform_buffer.allocation);
}

void Renderer::CreateDescriptorSet()
{
	vk::DescriptorSetLayout layouts[] = { descriptor_set_layout };

	auto alloc_info = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(descriptor_pool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(layouts);

	descriptor_set = *engine->GetVkDevice().allocateDescriptorSets(alloc_info).begin();

	auto buffer_info = vk::DescriptorBufferInfo()
		.setBuffer(matrix_uniform_buffer.buffer)
		.setOffset(0)
		.setRange(sizeof(MatrixUniformBuffer));

	auto buffer_write = vk::WriteDescriptorSet()
		.setDstSet(descriptor_set)
		.setDstBinding(0)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setPBufferInfo(&buffer_info);

	engine->GetVkDevice().updateDescriptorSets({buffer_write}, nullptr);
}


void Renderer::AddMaterial(Material *material)
{
	for(auto pipeline : material_pipelines)
	{
		if(pipeline.material == material)
			return;
	}

	material_pipelines.push_back(CreateMaterialPipeline(material));
}

void Renderer::RemoveMaterial(Material *material)
{
	for(auto it=material_pipelines.begin(); it!=material_pipelines.end(); it++)
	{
		if(it->material == material)
		{
			DestroyMaterialPipeline(*it);
			material_pipelines.erase(it);
			return;
		}
	}
}

vk::ShaderModule CreateShaderModule(vk::Device device, const std::vector<char> &code)
{
	return device.createShaderModule(
		vk::ShaderModuleCreateInfo()
			.setCodeSize(code.size())
			.setPCode(reinterpret_cast<const uint32_t *>(code.data())));
}

Renderer::MaterialPipeline Renderer::CreateMaterialPipeline(Material *material)
{
	auto device = engine->GetVkDevice();

	auto pipeline = MaterialPipeline(material);

	auto vert_shader_module = CreateShaderModule(device, ReadSPIRVShader("texture.vert"));
	auto frag_shader_module = CreateShaderModule(device, ReadSPIRVShader("texture.frag"));

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

	vk::Viewport viewport(0.0f, 0.0f, screen_extent.width, screen_extent.height, 0.0f, 1.0f);
	vk::Rect2D scissor({0, 0}, screen_extent);

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


	std::array<vk::DescriptorSetLayout, 2> descriptor_set_layouts = {
		descriptor_set_layout, material->GetDescriptorSetLayout()
	};

	auto pipeline_layout_info = vk::PipelineLayoutCreateInfo()
		.setSetLayoutCount(descriptor_set_layouts.size())
		.setPSetLayouts(descriptor_set_layouts.data());

	pipeline.pipeline_layout = engine->GetVkDevice().createPipelineLayout(pipeline_layout_info);


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
		.setLayout(pipeline.pipeline_layout)
		.setRenderPass(render_pass)
		.setSubpass(0);


	pipeline.pipeline = engine->GetVkDevice().createGraphicsPipeline(vk::PipelineCache() /*nullptr*/, pipeline_info);


	engine->GetVkDevice().destroyShaderModule(vert_shader_module);
	engine->GetVkDevice().destroyShaderModule(frag_shader_module);

	return pipeline;
}


void Renderer::DestroyMaterialPipeline(const Renderer::MaterialPipeline &material_pipeline)
{
	auto &device = engine->GetVkDevice();
	device.destroyPipeline(material_pipeline.pipeline);
	device.destroyPipelineLayout(material_pipeline.pipeline_layout);
}

void Renderer::RecreateAllMaterialPipelines()
{
	for(auto &material_pipeline : material_pipelines)
	{
		auto material = material_pipeline.material;
		DestroyMaterialPipeline(material_pipeline);
		material_pipeline = CreateMaterialPipeline(material);
	}
}

void Renderer::CreateDepthResources()
{
	depth_format = engine->FindDepthFormat();

	depth_image = engine->Create2DImage(screen_extent.width, screen_extent.height, depth_format,
										vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
										VMA_MEMORY_USAGE_GPU_ONLY);

	depth_image_view = engine->GetVkDevice().createImageView(vk::ImageViewCreateInfo()
																 .setImage(depth_image.image)
																 .setViewType(vk::ImageViewType::e2D)
																 .setFormat(depth_format)
																 .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1)));

	engine->TransitionImageLayout(depth_image.image, depth_format, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

void Renderer::CleanupDepthResources()
{
	engine->GetVkDevice().destroyImageView(depth_image_view);
	engine->DestroyImage(depth_image);
}

void Renderer::CreateRenderPasses()
{
	auto color_attachment = vk::AttachmentDescription()
		.setFormat(format)
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

void Renderer::CleanupRenderPasses()
{
	engine->GetVkDevice().destroyRenderPass(render_pass);
}


void Renderer::ResizeScreen(vk::Extent2D screen_extent, std::vector<vk::ImageView> dst_image_views)
{
	this->screen_extent = screen_extent;
	this->dst_image_views = dst_image_views;

	//CleanupRenderPasses();
	//CreateRenderPasses();

	RecreateAllMaterialPipelines();

	CleanupDepthResources();
	CreateDepthResources();

	CleanupFramebuffers();
	CreateFramebuffers();

	CleanupCommandBuffers();
	CreateCommandBuffers();
}

void Renderer::CreateCommandBuffers()
{
	auto pipeline = material_pipelines[0];

	command_buffers = engine->GetVkDevice().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
					.setCommandPool(command_pool)
					.setLevel(vk::CommandBufferLevel::ePrimary)
					.setCommandBufferCount(static_cast<uint32_t>(dst_image_views.size())));

	for(size_t i=0; i<command_buffers.size(); i++)
	{
		const auto &command_buffer = command_buffers[i];

		command_buffer.begin({ vk::CommandBufferUsageFlagBits::eSimultaneousUse });

		std::array<vk::ClearValue, 2> clear_values = {
			vk::ClearColorValue(std::array<float, 4>{{0.0f, 0.0f, 0.0f, 1.0f }}),
			vk::ClearDepthStencilValue(1.0f, 0)
		};

		command_buffer.beginRenderPass(
				vk::RenderPassBeginInfo()
					.setRenderPass(render_pass)
					.setFramebuffer(dst_framebuffers[i])
					.setRenderArea(vk::Rect2D({0, 0 }, screen_extent))
					.setClearValueCount(clear_values.size())
					.setPClearValues(clear_values.data()),
				vk::SubpassContents::eInline);


		command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline);
		command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.pipeline_layout, 0, descriptor_set, nullptr);
		command_buffer.bindVertexBuffers(0, { test_mesh->vertex_buffer.buffer }, { 0 });
		command_buffer.bindIndexBuffer(test_mesh->index_buffer.buffer, 0, vk::IndexType::eUint16);

		for(auto primitive : test_mesh->primitives)
		{
			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.pipeline_layout, 1, primitive.material_instance->GetDescriptorSet(), nullptr);
			command_buffer.drawIndexed(primitive.indices_count, 1, primitive.indices_offset, 0, 0);
		}

		command_buffer.endRenderPass();

		command_buffer.end();
	}
}

void Renderer::CleanupCommandBuffers()
{
	engine->GetVkDevice().freeCommandBuffers(command_pool, command_buffers);
}

void Renderer::DrawFrame(std::uint32_t image_index, std::vector<vk::Semaphore> wait_semaphores,
						 std::vector<vk::PipelineStageFlags> wait_stages, std::vector<vk::Semaphore> signal_semaphores)
{
	engine->GetGraphicsQueue().submit(
		vk::SubmitInfo()
			.setWaitSemaphoreCount(static_cast<uint32_t>(wait_semaphores.size()))
			.setPWaitSemaphores(wait_semaphores.data())
			.setPWaitDstStageMask(wait_stages.data())
			.setCommandBufferCount(1)
			.setPCommandBuffers(&command_buffers[image_index])
			.setSignalSemaphoreCount(static_cast<uint32_t>(signal_semaphores.size()))
			.setPSignalSemaphores(signal_semaphores.data()),
		nullptr);
}
