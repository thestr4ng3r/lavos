
#include <chrono>
#include <iostream>

#include "lavos/glm_config.h"
#include "lavos/component/directional_light_component.h"
#include "lavos/renderer.h"
#include "lavos/shader_load.h"
#include "lavos/vertex.h"
#include "lavos/component/mesh_component.h"

using namespace lavos;

Renderer::Renderer(Engine *engine, ColorRenderTarget *color_render_target, DepthRenderTarget *depth_render_target)
	: engine(engine)
{
	this->color_render_target = color_render_target;
	this->depth_render_target = depth_render_target;
	color_render_target->AddChangedCallback(RenderTarget::ChangedCallbackOrder::Renderer, this);

	CreateDescriptorPool();
	CreateDescriptorSetLayout();
	CreateUniformBuffers();
	CreateDescriptorSet();

	CreateRenderPasses();

	CreateFramebuffers();

	CreateRenderCommandPool();
	CreateRenderCommandBuffer();
}

Renderer::~Renderer()
{
	color_render_target->RemoveChangedCallback(this);

	auto &device = engine->GetVkDevice();

	device.destroyDescriptorSetLayout(descriptor_set_layout);

	for(auto &material_pipeline : material_pipelines)
		DestroyMaterialPipeline(material_pipeline);

	device.destroyDescriptorPool(descriptor_pool);

	engine->DestroyBuffer(matrix_uniform_buffer);
	engine->DestroyBuffer(lighting_uniform_buffer);
	engine->DestroyBuffer(camera_uniform_buffer);

	CleanupFramebuffers();
	CleanupRenderCommandPool();

	CleanupRenderPasses();
}

void Renderer::CreateRenderCommandPool()
{
	auto queue_family_indices = engine->GetQueueFamilyIndices();

	auto command_pool_info = vk::CommandPoolCreateInfo()
		.setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		.setQueueFamilyIndex(static_cast<uint32_t>(queue_family_indices.graphics_family));

	render_command_pool = engine->GetVkDevice().createCommandPool(command_pool_info);
}

void Renderer::CleanupRenderCommandPool()
{
	engine->GetVkDevice().destroyCommandPool(render_command_pool);
}

void Renderer::CreateFramebuffers()
{
	auto dst_image_views = color_render_target->GetImageViews();
	auto extent = color_render_target->GetExtent();

	dst_framebuffers.resize(dst_image_views.size());

	for(size_t i=0; i<dst_image_views.size(); i++)
	{
		std::array<vk::ImageView, 2> attachments = {
			dst_image_views[i],
			depth_render_target->GetImageView()
		};

		auto framebuffer_info = vk::FramebufferCreateInfo()
			.setRenderPass(render_pass)
			.setAttachmentCount(attachments.size())
			.setPAttachments(attachments.data())
			.setWidth(extent.width)
			.setHeight(extent.height)
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
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 3)
	};

	auto create_info = vk::DescriptorPoolCreateInfo()
		.setPoolSizeCount(static_cast<uint32_t>(pool_sizes.size()))
		.setPPoolSizes(pool_sizes.data())
		.setMaxSets(1);

	descriptor_pool = engine->GetVkDevice().createDescriptorPool(create_info);
}

void Renderer::CreateDescriptorSetLayout()
{
	std::vector<vk::DescriptorSetLayoutBinding> bindings = {
		// matrix
		vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex),

		// lighting
		vk::DescriptorSetLayoutBinding()
			.setBinding(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment),

		// camera
		vk::DescriptorSetLayoutBinding()
			.setBinding(2)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
	};

	auto create_info = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(static_cast<uint32_t>(bindings.size()))
		.setPBindings(bindings.data());

	descriptor_set_layout = engine->GetVkDevice().createDescriptorSetLayout(create_info);
}


void Renderer::CreateUniformBuffers()
{
	matrix_uniform_buffer = engine->CreateBuffer(sizeof(MatrixUniformBuffer),
												 vk::BufferUsageFlagBits::eUniformBuffer,
												 VMA_MEMORY_USAGE_CPU_ONLY);

	lighting_uniform_buffer = engine->CreateBuffer(sizeof(LightingUniformBuffer),
												   vk::BufferUsageFlagBits::eUniformBuffer,
												   VMA_MEMORY_USAGE_CPU_ONLY);

	camera_uniform_buffer = engine->CreateBuffer(sizeof(CameraUniformBuffer),
												 vk::BufferUsageFlagBits::eUniformBuffer,
												 VMA_MEMORY_USAGE_CPU_ONLY);
}

void Renderer::UpdateMatrixUniformBuffer()
{
	auto extent = color_render_target->GetExtent();

	if(auto_set_camera_aspect && camera->GetType() == CameraComponent::Type::PERSPECTIVE)
		camera->SetPerspectiveAspect((float)extent.width / (float)extent.height);

	MatrixUniformBuffer matrix_ubo;
	matrix_ubo.modelview = camera->GetModelViewMatrix();
	matrix_ubo.projection = camera->GetProjectionMatrix();
	matrix_ubo.projection[1][1] *= -1.0f;

	void *data = engine->MapMemory(matrix_uniform_buffer.allocation);
	memcpy(data, &matrix_ubo, sizeof(matrix_ubo));
	engine->UnmapMemory(matrix_uniform_buffer.allocation);
}

void Renderer::UpdateCameraUniformBuffer()
{
	CameraUniformBuffer ubo;
	ubo.position = camera->GetNode()->GetTransformComponent()->GetMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	void *data = engine->MapMemory(camera_uniform_buffer.allocation);
	memcpy(data, &ubo, sizeof(ubo));
	engine->UnmapMemory(camera_uniform_buffer.allocation);
}

void Renderer::UpdateLightingUniformBuffer()
{
	LightingUniformBuffer ubo;
	memset(&ubo, 0, sizeof(ubo));

	ubo.ambient_intensity = scene->GetAmbientLightIntensity();

	auto dir_light = scene->GetRootNode()->GetComponentInChildren<DirectionalLightComponent>();
	if(dir_light != nullptr)
	{
		ubo.directional_light_enabled = 1;
		ubo.directional_light_dir = glm::normalize(dir_light->GetNode()->GetTransformComponent()->GetMatrixWorld()
												   * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
		ubo.directional_light_intensity = dir_light->GetIntensity();
	}
	else
	{
		ubo.directional_light_enabled = 0;
	}

	void *data = engine->MapMemory(lighting_uniform_buffer.allocation);
	memcpy(data, &ubo, sizeof(ubo));
	engine->UnmapMemory(lighting_uniform_buffer.allocation);
}


void Renderer::CreateDescriptorSet()
{
	vk::DescriptorSetLayout layouts[] = { descriptor_set_layout };

	auto alloc_info = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(descriptor_pool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(layouts);

	descriptor_set = *engine->GetVkDevice().allocateDescriptorSets(alloc_info).begin();

	auto matrix_buffer_info = vk::DescriptorBufferInfo()
		.setBuffer(matrix_uniform_buffer.buffer)
		.setOffset(0)
		.setRange(sizeof(MatrixUniformBuffer));

	auto matrix_buffer_write = vk::WriteDescriptorSet()
		.setDstSet(descriptor_set)
		.setDstBinding(0)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setPBufferInfo(&matrix_buffer_info);


	auto lighting_buffer_info = vk::DescriptorBufferInfo()
		.setBuffer(lighting_uniform_buffer.buffer)
		.setOffset(0)
		.setRange(sizeof(LightingUniformBuffer));

	auto lighting_buffer_write = vk::WriteDescriptorSet()
		.setDstSet(descriptor_set)
		.setDstBinding(1)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setPBufferInfo(&lighting_buffer_info);


	auto camera_buffer_info = vk::DescriptorBufferInfo()
		.setBuffer(camera_uniform_buffer.buffer)
		.setOffset(0)
		.setRange(sizeof(CameraUniformBuffer));

	auto camera_buffer_write = vk::WriteDescriptorSet()
		.setDstSet(descriptor_set)
		.setDstBinding(2)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setPBufferInfo(&camera_buffer_info);

	engine->GetVkDevice().updateDescriptorSets({matrix_buffer_write, lighting_buffer_write, camera_buffer_write}, nullptr);
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


Renderer::MaterialPipeline Renderer::CreateMaterialPipeline(Material *material)
{
	auto device = engine->GetVkDevice();

	auto pipeline = MaterialPipeline(material);

	auto shader_stages = material->GetShaderStageCreateInfos();

	auto vertex_binding_description = Vertex::GetBindingDescription();
	auto vertex_attribute_description = Vertex::GetAttributeDescription();

	auto vertex_input_info = vk::PipelineVertexInputStateCreateInfo()
		.setVertexBindingDescriptionCount(1)
		.setPVertexBindingDescriptions(&vertex_binding_description)
		.setVertexAttributeDescriptionCount(vertex_attribute_description.size())
		.setPVertexAttributeDescriptions(vertex_attribute_description.data());

	auto input_assembly_info = vk::PipelineInputAssemblyStateCreateInfo()
		.setTopology(material->GetPrimitiveTopology());

	auto extent = color_render_target->GetExtent();
	vk::Viewport viewport(0.0f, 0.0f, extent.width, extent.height, 0.0f, 1.0f);
	vk::Rect2D scissor({0, 0}, extent);

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



	std::vector<vk::DescriptorSetLayout> descriptor_set_layouts = {
		descriptor_set_layout
	};

	pipeline.renderer_descriptor_set_index = 0;

	auto material_descriptor_set_layout = material->GetDescriptorSetLayout();
	if(material_descriptor_set_layout)
	{
		descriptor_set_layouts.push_back(material_descriptor_set_layout);
		pipeline.material_descriptor_set_index = 1;
	}
	else
	{
		pipeline.material_descriptor_set_index = -1;
	}


	auto push_constant_range = vk::PushConstantRange()
		.setStageFlags(vk::ShaderStageFlagBits::eVertex)
		.setOffset(0)
		.setSize(sizeof(TransformPushConstant));


	auto pipeline_layout_info = vk::PipelineLayoutCreateInfo()
		.setSetLayoutCount(static_cast<uint32_t>(descriptor_set_layouts.size()))
		.setPSetLayouts(descriptor_set_layouts.data())
		.setPushConstantRangeCount(1)
		.setPPushConstantRanges(&push_constant_range);

	pipeline.pipeline_layout = device.createPipelineLayout(pipeline_layout_info);


	auto pipeline_info = vk::GraphicsPipelineCreateInfo()
		.setStageCount(static_cast<uint32_t>(shader_stages.size()))
		.setPStages(shader_stages.data())
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


	pipeline.pipeline = device.createGraphicsPipeline(nullptr, pipeline_info);

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

void Renderer::CreateRenderPasses()
{
	auto color_attachment = vk::AttachmentDescription()
		.setFormat(color_render_target->GetFormat())
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
	vk::AttachmentReference color_attachment_ref(0, vk::ImageLayout::eColorAttachmentOptimal);

	auto depth_attachment = vk::AttachmentDescription()
		.setFormat(depth_render_target->GetFormat())
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

void Renderer::CreateRenderCommandBuffer()
{
	render_command_buffer = engine->GetVkDevice().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
					.setCommandPool(render_command_pool)
					.setLevel(vk::CommandBufferLevel::ePrimary)
					.setCommandBufferCount(1)).front();
}



void Renderer::CleanupRenderCommandBuffer()
{
	engine->GetVkDevice().freeCommandBuffers(render_command_pool, render_command_buffer);
}

void Renderer::RecordRenderCommandBuffer(vk::CommandBuffer command_buffer, vk::Framebuffer dst_framebuffer)
{
	// TODO: here is a LOT of potential for optimization

	std::array<vk::ClearValue, 2> clear_values = {
		vk::ClearColorValue(std::array<float, 4>{{0.0f, 0.0f, 0.0f, 1.0f }}),
		vk::ClearDepthStencilValue(1.0f, 0)
	};

	auto extent = color_render_target->GetExtent();

	command_buffer.beginRenderPass(
		vk::RenderPassBeginInfo()
			.setRenderPass(render_pass)
			.setFramebuffer(dst_framebuffer)
			.setRenderArea(vk::Rect2D({0, 0 }, extent))
			.setClearValueCount(clear_values.size())
			.setPClearValues(clear_values.data()),
		vk::SubpassContents::eInline);

	auto pipeline = material_pipelines[0];

	command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline);
	command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
									  pipeline.pipeline_layout,
									  static_cast<uint32_t>(pipeline.renderer_descriptor_set_index),
									  descriptor_set,
									  nullptr);

	scene->GetRootNode()->TraversePreOrder([command_buffer, pipeline] (Node *node) {
		auto mesh_component = node->GetComponent<MeshComponent>();
		if(mesh_component == nullptr)
			return;

		auto mesh = mesh_component->GetMesh();
		if(mesh == nullptr)
			return;

		command_buffer.bindVertexBuffers(0, { mesh->vertex_buffer.buffer }, { 0 });
		command_buffer.bindIndexBuffer(mesh->index_buffer.buffer, 0, vk::IndexType::eUint16);

		auto transform_component = node->GetTransformComponent();
		TransformPushConstant transform_push_constant;
		if(transform_component != nullptr)
			transform_push_constant.transform = transform_component->GetMatrixWorld();

		command_buffer.pushConstants(pipeline.pipeline_layout,
									 vk::ShaderStageFlagBits::eVertex,
									 0,
									 sizeof(TransformPushConstant),
									 &transform_push_constant);

		for(auto primitive : mesh->primitives)
		{
			if(pipeline.material_descriptor_set_index >= 0)
			{
				auto descriptor_set = primitive.material_instance->GetDescriptorSet();
				command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
												  pipeline.pipeline_layout,
												  static_cast<uint32_t>(pipeline.material_descriptor_set_index),
												  descriptor_set,
												  nullptr);
			}

			command_buffer.drawIndexed(primitive.indices_count, 1, primitive.indices_offset, 0, 0);
		}
	});


	command_buffer.endRenderPass();
}

void Renderer::DrawFrame(std::uint32_t image_index, std::vector<vk::Semaphore> wait_semaphores,
						 std::vector<vk::PipelineStageFlags> wait_stages, std::vector<vk::Semaphore> signal_semaphores)
{
	render_command_buffer.begin({ vk::CommandBufferUsageFlagBits::eSimultaneousUse });
	DrawFrameRecord(render_command_buffer, dst_framebuffers[image_index]);
	render_command_buffer.end();

	engine->GetGraphicsQueue().submit(
		vk::SubmitInfo()
			.setWaitSemaphoreCount(static_cast<uint32_t>(wait_semaphores.size()))
			.setPWaitSemaphores(wait_semaphores.data())
			.setPWaitDstStageMask(wait_stages.data())
			.setCommandBufferCount(1)
			.setPCommandBuffers(&render_command_buffer)
			.setSignalSemaphoreCount(static_cast<uint32_t>(signal_semaphores.size()))
			.setPSignalSemaphores(signal_semaphores.data()),
		nullptr);
}

void Renderer::DrawFrameRecord(vk::CommandBuffer command_buffer, vk::Framebuffer dst_framebuffer)
{
	if(scene == nullptr)
		throw std::runtime_error("renderer has no scene.");

	if(camera == nullptr)
		throw std::runtime_error("renderer has no camera.");

	UpdateMatrixUniformBuffer();
	UpdateLightingUniformBuffer();
	UpdateCameraUniformBuffer();

	RecordRenderCommandBuffer(command_buffer, dst_framebuffer);
}

void Renderer::RenderTargetChanged(RenderTarget *render_target)
{
	//CleanupRenderPasses();
	//CreateRenderPasses();

	RecreateAllMaterialPipelines(); // TODO: really necessary?

	CleanupFramebuffers();
	CreateFramebuffers();
}
