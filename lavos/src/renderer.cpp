
#include <chrono>
#include <iostream>

#include "lavos/glm_config.h"
#include "lavos/component/directional_light.h"
#include "lavos/component/spot_light.h"
#include "lavos/spot_light_shadow.h"
#include "lavos/renderer.h"
#include "lavos/shader_load.h"
#include "lavos/vertex.h"
#include "lavos/component/mesh_component.h"
#include "lavos/sub_renderer.h"
#include "lavos/vk_util.h"

#include "../glsl/common_glsl_cpp.h"

using namespace lavos;

Renderer::Renderer(Engine *engine,
				   const RenderConfig &config,
				   ColorRenderTarget *color_render_target,
				   DepthRenderTarget *depth_render_target)
	: engine(engine), config(config)
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

	material_pipeline_manager = new MaterialPipelineManager(engine, CreateMaterialPipelineConfiguration());

	CreateRenderCommandBuffer();
}

Renderer::~Renderer()
{
	color_render_target->RemoveChangedCallback(this);

	for(auto sub_renderer : sub_renderers)
		delete sub_renderer;

	auto &device = engine->GetVkDevice();

	device.destroyDescriptorSetLayout(descriptor_set_layout);

	delete material_pipeline_manager;

	device.destroyDescriptorPool(descriptor_pool);

	delete matrix_uniform_buffer;
	delete lighting_uniform_buffer;
	delete camera_uniform_buffer;

	CleanupFramebuffers();

	CleanupRenderPasses();
}

MaterialPipelineConfiguration Renderer::CreateMaterialPipelineConfiguration()
{
	auto color_blend_attachment = vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(vk::ColorComponentFlagBits::eR
							   | vk::ColorComponentFlagBits::eG
							   | vk::ColorComponentFlagBits::eB
							   | vk::ColorComponentFlagBits::eA)
			.setBlendEnable(VK_FALSE);

	auto color_blend_state_info = vk_util::PipelineColorBlendStateCreateInfo()
			.SetAttachments({ color_blend_attachment });

	return MaterialPipelineConfiguration(
			color_render_target->GetExtent(),
			descriptor_set_layout,
			render_pass,
			Material::DefaultRenderMode::ColorForward,
			color_blend_state_info);
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
	std::array<vk::DescriptorSetLayoutBinding, 3> bindings = {
		// matrix
		vk::DescriptorSetLayoutBinding()
			.setBinding(DESCRIPTOR_SET_COMMON_BINDING_MATRIX_BUFFER)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex),

		// lighting
		vk::DescriptorSetLayoutBinding()
			.setBinding(DESCRIPTOR_SET_COMMON_BINDING_LIGHTING_BUFFER)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment),

		// camera
		vk::DescriptorSetLayoutBinding()
			.setBinding(DESCRIPTOR_SET_COMMON_BINDING_CAMERA_BUFFER)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
	};

	auto create_info = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(static_cast<uint32_t>(bindings.size()))
		.setPBindings(bindings.data());

	descriptor_set_layout = engine->GetVkDevice().createDescriptorSetLayout(create_info);
}


size_t Renderer::GetLightingUniformBufferSize()
{
	return sizeof(LightingUniformBufferFixed)
		   + max_spot_lights * sizeof(LightingUniformBufferSpotLight);
}

void Renderer::CreateUniformBuffers()
{
	matrix_uniform_buffer = engine->CreateBuffer(sizeof(MatrixUniformBuffer),
												 vk::BufferUsageFlagBits::eUniformBuffer,
												 VMA_MEMORY_USAGE_CPU_ONLY);

	lighting_uniform_buffer = engine->CreateBuffer(GetLightingUniformBufferSize(),
												   vk::BufferUsageFlagBits::eUniformBuffer,
												   VMA_MEMORY_USAGE_CPU_ONLY);

	camera_uniform_buffer = engine->CreateBuffer(sizeof(CameraUniformBuffer),
												 vk::BufferUsageFlagBits::eUniformBuffer,
												 VMA_MEMORY_USAGE_CPU_ONLY);
}

void Renderer::UpdateMatrixUniformBuffer()
{
	auto extent = color_render_target->GetExtent();

	if(auto_set_camera_aspect && camera->GetType() == Camera::Type::PERSPECTIVE)
		camera->SetPerspectiveAspect((float)extent.width / (float)extent.height);

	MatrixUniformBuffer matrix_ubo;
	matrix_ubo.modelview = camera->GetModelViewMatrix();
	matrix_ubo.projection = camera->GetProjectionMatrix();
	matrix_ubo.projection[1][1] *= -1.0f;

	memcpy(matrix_uniform_buffer->Map(), &matrix_ubo, sizeof(matrix_ubo));
	matrix_uniform_buffer->UnMap();
}

void Renderer::UpdateCameraUniformBuffer()
{
	CameraUniformBuffer ubo;
	ubo.position = camera->GetNode()->GetTransformComp()->GetMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	memcpy(camera_uniform_buffer->Map(), &ubo, sizeof(ubo));
	camera_uniform_buffer->UnMap();
}


void Renderer::UpdateLightingUniformBuffer()
{
	LightingUniformBufferFixed fixed;
	memset(&fixed, 0, sizeof(fixed));

	fixed.ambient_intensity = scene->GetAmbientLightIntensity();

	auto dir_light = scene->GetRootNode()->GetComponentInChildren<DirectionalLight>();
	if(dir_light != nullptr)
	{
		fixed.directional_light_enabled = 1;
		fixed.directional_light_dir = glm::normalize(dir_light->GetNode()->GetTransformComp()->GetMatrixWorld()
													 * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
		fixed.directional_light_intensity = dir_light->GetIntensity();
	}
	else
	{
		fixed.directional_light_enabled = 0;
	}


	std::vector<LightingUniformBufferSpotLight> spot_light_buffers(max_spot_lights);
	memset(spot_light_buffers.data(), 0, sizeof(LightingUniformBufferSpotLight) * spot_light_buffers.size());

	auto spot_lights = scene->GetRootNode()->GetComponentsInChildren<SpotLight>();

	fixed.spot_lights_count = static_cast<uint32_t>(spot_lights.size());

	for(unsigned int i=0; i<std::min(spot_lights.size(), spot_light_buffers.size()); i++)
	{
		auto spot_light = spot_lights[i];
		glm::mat4 transform_mat = spot_light->GetNode()->GetTransformComp()->GetMatrixWorld();
		spot_light_buffers[i].position = transform_mat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		spot_light_buffers[i].direction = transform_mat * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
		spot_light_buffers[i].angle_cos = cosf(spot_light->GetAngle() * 0.5f);
	}

	std::uint8_t *data = static_cast<std::uint8_t *>(lighting_uniform_buffer->Map());
	memcpy(data, &fixed, sizeof(fixed));
	memcpy(data + 48, spot_light_buffers.data(), sizeof(LightingUniformBufferSpotLight) * spot_light_buffers.size());
	lighting_uniform_buffer->UnMap();
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
		.setBuffer(matrix_uniform_buffer->GetVkBuffer())
		.setOffset(0)
		.setRange(sizeof(MatrixUniformBuffer));

	auto matrix_buffer_write = vk::WriteDescriptorSet()
		.setDstSet(descriptor_set)
		.setDstBinding(DESCRIPTOR_SET_COMMON_BINDING_MATRIX_BUFFER)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setPBufferInfo(&matrix_buffer_info);


	auto lighting_buffer_info = vk::DescriptorBufferInfo()
		.setBuffer(lighting_uniform_buffer->GetVkBuffer())
		.setOffset(0)
		.setRange(GetLightingUniformBufferSize());

	auto lighting_buffer_write = vk::WriteDescriptorSet()
		.setDstSet(descriptor_set)
		.setDstBinding(DESCRIPTOR_SET_COMMON_BINDING_LIGHTING_BUFFER)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setPBufferInfo(&lighting_buffer_info);


	auto camera_buffer_info = vk::DescriptorBufferInfo()
		.setBuffer(camera_uniform_buffer->GetVkBuffer())
		.setOffset(0)
		.setRange(sizeof(CameraUniformBuffer));

	auto camera_buffer_write = vk::WriteDescriptorSet()
		.setDstSet(descriptor_set)
		.setDstBinding(DESCRIPTOR_SET_COMMON_BINDING_CAMERA_BUFFER)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setPBufferInfo(&camera_buffer_info);

	engine->GetVkDevice().updateDescriptorSets({matrix_buffer_write, lighting_buffer_write, camera_buffer_write}, nullptr);
}


void Renderer::AddSubRenderer(SubRenderer *sub_renderer)
{
	if(std::find(sub_renderers.begin(), sub_renderers.end(), sub_renderer) != sub_renderers.end())
		return;

	sub_renderers.push_back(sub_renderer);

	for(auto material : materials)
		sub_renderer->AddMaterial(material);
}

void Renderer::RemoveSubRenderer(SubRenderer *sub_renderer)
{
	auto it = std::find(sub_renderers.begin(), sub_renderers.end(), sub_renderer);
	if(it == sub_renderers.end())
		throw std::runtime_error("SubRenderer not in Renderer.");

	for(auto material : materials)
		sub_renderer->RemoveMaterial(material);
}

void Renderer::AddMaterial(Material *material)
{
	if(std::find(materials.begin(), materials.end(), material) != materials.end())
		return;

	materials.push_back(material);
	material_pipeline_manager->AddMaterial(material);

	for(auto sub_renderer : sub_renderers)
		sub_renderer->AddMaterial(material);
}


void Renderer::RemoveMaterial(Material *material)
{
	auto it = std::find(materials.begin(), materials.end(), material);
	if(it == materials.end())
		throw std::runtime_error("Material not in Renderer.");

	material_pipeline_manager->RemoveMaterial(material);

	for(auto sub_renderer : sub_renderers)
		sub_renderer->RemoveMaterial(material);

	materials.erase(it);
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

	vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), render_pass, "Renderer RenderPass");
}

void Renderer::CleanupRenderPasses()
{
	engine->GetVkDevice().destroyRenderPass(render_pass);
}



void Renderer::CreateRenderCommandBuffer()
{
	render_command_buffer = engine->GetVkDevice().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
					.setCommandPool(engine->GetRenderCommandPool())
					.setLevel(vk::CommandBufferLevel::ePrimary)
					.setCommandBufferCount(1)).front();
}

void Renderer::CleanupRenderCommandBuffer()
{
	engine->GetVkDevice().freeCommandBuffers(engine->GetRenderCommandPool(), render_command_buffer);
}

void Renderer::RecordRenderables(vk::CommandBuffer command_buffer,
		Material::RenderMode render_mode,
		MaterialPipelineManager *material_pipeline_manager,
		vk::DescriptorSet renderer_descriptor_set)
{
	std::multimap<Material *, std::set<std::pair<Node *, Renderable *>>> material_primitives;

	scene->GetRootNode()->TraversePreOrder([&material_primitives] (Node *node) {
		auto renderable = node->GetComponent<Renderable>();
		if(renderable == nullptr || !renderable->GetCurrentlyRenderable())
			return;

		unsigned int primitives_count = renderable->GetPrimitivesCount();
		for(unsigned int i=0; i<primitives_count; i++)
		{
			auto primitive = renderable->GetPrimitive(i);
			auto material = primitive->GetMaterialInstance()->GetMaterial();

			auto it = material_primitives.find(material);
			if(it != material_primitives.end())
				it->second.insert(std::pair<Node *, Renderable *>(node, renderable));
			else
				material_primitives.insert(std::pair<Material *, std::set<std::pair<Node *, Renderable *>>>(
						material, { std::pair<Node *, Renderable *>(node, renderable) }));
		}
	});

	for(auto &entry : material_primitives)
	{
		auto material = entry.first;
		auto &renderables = entry.second;

		MaterialPipeline *pipeline = material_pipeline_manager->GetMaterialPipeline(material);
		if(!pipeline)
			continue;

		int material_descriptor_set_index = pipeline->material_descriptor_set_index;
		auto pipeline_layout = pipeline->pipeline_layout;

		command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->pipeline);
		command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
										  pipeline_layout,
										  static_cast<uint32_t>(pipeline->renderer_descriptor_set_index),
										  renderer_descriptor_set,
										  nullptr);


		for(auto &renderable_entry : renderables)
		{
			auto node = renderable_entry.first;
			auto renderable = renderable_entry.second;

			auto transform_component = node->GetTransformComp();
			TransformPushConstant transform_push_constant;
			if(transform_component != nullptr)
				transform_push_constant.transform = transform_component->GetMatrixWorld();

			command_buffer.pushConstants(pipeline_layout,
										 vk::ShaderStageFlagBits::eVertex,
										 0,
										 sizeof(TransformPushConstant),
										 &transform_push_constant);

			renderable->BindBuffers(command_buffer);

			unsigned int primitives_count = renderable->GetPrimitivesCount();
			for(unsigned int i=0; i<primitives_count; i++)
			{
				auto primitive = renderable->GetPrimitive(i);

				auto material_instance = primitive->GetMaterialInstance();
				if(material_instance->GetMaterial() != material)
					continue;

				if(material_descriptor_set_index >= 0)
				{
					auto descriptor_set = primitive->GetMaterialInstance()->GetDescriptorSet(render_mode);
					command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
													  pipeline_layout,
													  static_cast<uint32_t>(material_descriptor_set_index),
													  descriptor_set,
													  nullptr);
				}

				primitive->Draw(command_buffer);
			}
		}
	}
}

void Renderer::DrawFrame(std::uint32_t image_index, std::vector<vk::Semaphore> wait_semaphores,
						 std::vector<vk::PipelineStageFlags> wait_stages, std::vector<vk::Semaphore> signal_semaphores)
{
	if(scene == nullptr)
		throw std::runtime_error("renderer has no scene.");

	if(camera == nullptr)
		throw std::runtime_error("renderer has no camera.");

	UpdateMatrixUniformBuffer();
	UpdateLightingUniformBuffer();
	UpdateCameraUniformBuffer();

	std::vector<SpotLight *> spot_lights = scene->GetRootNode()->GetComponentsInChildren<SpotLight>();
	std::vector<SpotLightShadow *> spot_light_shadows;

	std::vector<vk::Semaphore> wait_semaphores_internal;

	for(SpotLight *spot_light : spot_lights)
	{
		auto shadow = spot_light->GetShadow();
		if(!shadow)
			continue;
		auto command_buffer = shadow->BuildCommandBuffer(this);
		spot_light_shadows.push_back(shadow);
		wait_semaphores_internal.push_back(shadow->GetSemaphore());

		engine->GetVkDevice().waitIdle();
		engine->GetGraphicsQueue().submit(
			vk::SubmitInfo()
				.setWaitSemaphoreCount(0)
				.setPWaitSemaphores(nullptr)
				.setPWaitDstStageMask(nullptr)
				.setCommandBufferCount(1)
				.setPCommandBuffers(&command_buffer)
				.setSignalSemaphoreCount(0)
				.setPSignalSemaphores(nullptr),
			nullptr);
	}

	engine->GetVkDevice().waitIdle(); // TODO

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

	RecordRenderables(command_buffer,
			Material::DefaultRenderMode::ColorForward,
			material_pipeline_manager,
			descriptor_set);

	command_buffer.endRenderPass();
}

void Renderer::RenderTargetChanged(RenderTarget *render_target)
{
	//CleanupRenderPasses();
	//CreateRenderPasses();

	material_pipeline_manager->SetConfiguration(CreateMaterialPipelineConfiguration());

	CleanupFramebuffers();
	CreateFramebuffers();
}
