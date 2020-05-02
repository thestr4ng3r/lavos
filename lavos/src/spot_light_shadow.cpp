
#include "lavos/spot_light_shadow.h"
#include "lavos/component/spot_light.h"
#include "lavos/renderer.h"
#include "lavos/spot_light_shadow_renderer.h"

using namespace lavos;


struct ShadowMatrixUniformBuffer
{
	glm::mat4 modelview_projection;
};

static_assert(sizeof(ShadowMatrixUniformBuffer) == 64, "ShadowMatrixUniformBuffer memory layout");


SpotLightShadow::SpotLightShadow(Engine *engine, SpotLight *light, SpotLightShadowRenderer *renderer, float near_clip, float far_clip)
		: engine(engine), light(light), renderer(renderer), near_clip(near_clip), far_clip(far_clip)
{
	min_filter = vk::Filter::eLinear;
	mag_filter = vk::Filter::eLinear;

	CreateImage();
	CreateFramebuffer();
	CreateUniformBuffer();
	CreateDescriptorPool();
	CreateDescriptorSet();
}

glm::mat4 SpotLightShadow::GetModelViewMatrix()
{
	auto transform_component = light->GetNode()->GetTransformComp();
	if(transform_component == nullptr)
		throw std::runtime_error("node with a spot light component does not have a transform component.");

	glm::mat4 transform_mat = transform_component->GetMatrixWorld();
	return glm::inverse(transform_mat);
}

glm::mat4 SpotLightShadow::GetProjectionMatrix()
{
	auto r = glm::perspective(light->GetAngle(), 1.0f, near_clip, far_clip);
	r[1][1] *= -1.0f;
	return r;
}

SpotLightShadow::~SpotLightShadow()
{
	auto device = engine->GetVkDevice();

	device.free(engine->GetRenderCommandPool(), command_buffer);
	device.destroy(semaphore);
	engine->GetVkDevice().destroy(descriptor_pool);
	delete matrix_uniform_buffer;
	device.destroy(framebuffer);
	device.destroy(sampler);
	device.destroy(depth_image_view);
	engine->DestroyImage(depth_image);
	if(shadow_image)
	{
		device.destroy(shadow_image_view);
		engine->DestroyImage(shadow_image);
	}
	if(resolve_image)
	{
		device.destroy(resolve_image_view);
		engine->DestroyImage(resolve_image);
	}
}

void SpotLightShadow::CreateImage()
{
	bool have_shadow_tex = renderer->GetShadowFormat() != vk::Format::eUndefined;
	bool use_multisampling = renderer->GetSamples() != vk::SampleCountFlagBits::e1;

	vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	if(!have_shadow_tex)
		usage |= vk::ImageUsageFlagBits::eSampled;

	auto image_create_info = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setExtent(vk::Extent3D(renderer->GetWidth(), renderer->GetHeight(), 1))
			.setMipLevels(1)
			.setArrayLayers(1)
			.setSamples(renderer->GetSamples())
			.setTiling(vk::ImageTiling::eOptimal)
			.setFormat(renderer->GetDepthFormat())
			.setUsage(usage);

	depth_image = engine->CreateImage(image_create_info, VMA_MEMORY_USAGE_GPU_ONLY);
	vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), depth_image.image, "SpotLightShadow Depth Image");

	auto image_view_create_info = vk::ImageViewCreateInfo()
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(renderer->GetDepthFormat())
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1))
			.setImage(depth_image.image);

	depth_image_view = engine->GetVkDevice().createImageView(image_view_create_info);
	vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), depth_image_view, "SpotLightShadow Depth ImageView");

	if(have_shadow_tex)
	{
		usage = vk::ImageUsageFlagBits::eColorAttachment;
		if(!use_multisampling)
			usage |= vk::ImageUsageFlagBits::eSampled;
		else
			usage |= vk::ImageUsageFlagBits::eTransientAttachment;

		image_create_info = vk::ImageCreateInfo()
				.setImageType(vk::ImageType::e2D)
				.setExtent(vk::Extent3D(renderer->GetWidth(), renderer->GetHeight(), 1))
				.setMipLevels(1)
				.setArrayLayers(1)
				.setSamples(renderer->GetSamples())
				.setTiling(vk::ImageTiling::eOptimal)
				.setFormat(renderer->GetShadowFormat())
				.setUsage(usage);

		// TODO: use VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED if multisampling and available
		shadow_image = engine->CreateImage(image_create_info, VMA_MEMORY_USAGE_GPU_ONLY);
		vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), shadow_image.image, "SpotLightShadow Shadow Image");

		image_view_create_info = vk::ImageViewCreateInfo()
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(renderer->GetShadowFormat())
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
				.setImage(shadow_image.image);

		shadow_image_view = engine->GetVkDevice().createImageView(image_view_create_info);
		vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), shadow_image_view, "SpotLightShadow Shadow ImageView");

		if(use_multisampling)
		{
			image_create_info = vk::ImageCreateInfo()
					.setImageType(vk::ImageType::e2D)
					.setExtent(vk::Extent3D(renderer->GetWidth(), renderer->GetHeight(), 1))
					.setMipLevels(1)
					.setArrayLayers(1)
					.setSamples(vk::SampleCountFlagBits::e1)
					.setTiling(vk::ImageTiling::eOptimal)
					.setFormat(renderer->GetShadowFormat())
					.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);

			resolve_image = engine->CreateImage(image_create_info, VMA_MEMORY_USAGE_GPU_ONLY);
			vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), resolve_image.image, "SpotLightShadow Resolve Image");

			image_view_create_info = vk::ImageViewCreateInfo()
					.setViewType(vk::ImageViewType::e2D)
					.setFormat(renderer->GetShadowFormat())
					.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
					.setImage(resolve_image.image);

			resolve_image_view = engine->GetVkDevice().createImageView(image_view_create_info);
			vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), resolve_image_view, "SpotLightShadow Resolve ImageView");
		}
	}

	auto sampler_create_info = vk::SamplerCreateInfo()
			.setMagFilter(mag_filter)
			.setMinFilter(min_filter)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
			.setMipLodBias(0.0f)
			.setMaxAnisotropy(1.0f)
			.setMinLod(0.0f)
			.setMaxLod(1.0f)
			.setBorderColor(vk::BorderColor::eFloatOpaqueWhite);

	sampler = engine->GetVkDevice().createSampler(sampler_create_info);
	vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), sampler, "SpotLightShadow Sampler");
}

void SpotLightShadow::CreateFramebuffer()
{
	std::array<vk::ImageView, 3> attachments;
	attachments[0] = depth_image_view;
	size_t attachment_count = 1;

	if(shadow_image_view)
	{
		attachments[1] = { shadow_image_view };
		attachment_count++;

		if(resolve_image_view)
		{
			attachments[2] = { resolve_image_view };
			attachment_count++;
		}
	}

	auto create_info = vk::FramebufferCreateInfo()
			.setRenderPass(renderer->GetRenderPass())
			.setAttachmentCount(attachment_count)
			.setPAttachments(attachments.data())
			.setWidth(renderer->GetWidth())
			.setHeight(renderer->GetHeight())
			.setLayers(1);

	framebuffer = engine->GetVkDevice().createFramebuffer(create_info);
	vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), framebuffer, "SpotLightShadow");
}

void SpotLightShadow::CreateUniformBuffer()
{
	matrix_uniform_buffer = engine->CreateBuffer(sizeof(ShadowMatrixUniformBuffer),
												 vk::BufferUsageFlagBits::eUniformBuffer,
												 VMA_MEMORY_USAGE_CPU_ONLY);
	vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), matrix_uniform_buffer->GetVkBuffer(), "SpotLightShadow");
}

void SpotLightShadow::CreateDescriptorPool()
{
	std::array<vk::DescriptorPoolSize, 1> pool_sizes = {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1)
	};

	auto create_info = vk::DescriptorPoolCreateInfo()
			.setPoolSizeCount(static_cast<uint32_t>(pool_sizes.size()))
			.setPPoolSizes(pool_sizes.data())
			.setMaxSets(1);

	descriptor_pool = engine->GetVkDevice().createDescriptorPool(create_info);
	vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), descriptor_pool, "SpotLightShadow");
}

void SpotLightShadow::CreateDescriptorSet()
{
	vk::DescriptorSetLayout layouts[] = { renderer->GetDescriptorSetLayout() };

	auto alloc_info = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(descriptor_pool)
			.setDescriptorSetCount(1)
			.setPSetLayouts(layouts);

	descriptor_set = *engine->GetVkDevice().allocateDescriptorSets(alloc_info).begin();
	vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), descriptor_set, "SpotLightShadow");

	auto matrix_buffer_info = vk::DescriptorBufferInfo()
			.setBuffer(matrix_uniform_buffer->GetVkBuffer())
			.setOffset(0)
			.setRange(sizeof(ShadowMatrixUniformBuffer));

	auto matrix_buffer_write = vk::WriteDescriptorSet()
			.setDstSet(descriptor_set)
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setPBufferInfo(&matrix_buffer_info);

	engine->GetVkDevice().updateDescriptorSets(matrix_buffer_write, nullptr);
}

void SpotLightShadow::UpdateMatrixUniformBuffer()
{
	ShadowMatrixUniformBuffer matrix_ubo;
	matrix_ubo.modelview_projection = GetModelViewProjectionMatrix();
	memcpy(matrix_uniform_buffer->Map(), &matrix_ubo, sizeof(matrix_ubo));
	matrix_uniform_buffer->UnMap();
}


vk::CommandBuffer SpotLightShadow::BuildCommandBuffer(Renderer *renderer)
{
	const vk::Device &device = engine->GetVkDevice();

	UpdateMatrixUniformBuffer(); // TODO: Do this only if the contents really changed

	if(!command_buffer)
	{
		command_buffer = *engine->GetVkDevice().allocateCommandBuffers(
				vk::CommandBufferAllocateInfo(engine->GetRenderCommandPool(),
											  vk::CommandBufferLevel::ePrimary,
											  1)).begin();
		vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), command_buffer, "SpotLightShadow");
	}

	if(!semaphore)
	{
		semaphore = device.createSemaphore(vk::SemaphoreCreateInfo());
		vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), semaphore, "SpotLightShadow");
	}

	auto clear_value = vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0));

	auto render_pass_begin_info = vk::RenderPassBeginInfo()
			.setRenderPass(this->renderer->GetRenderPass())
			.setFramebuffer(framebuffer)
			.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(this->renderer->GetWidth(), this->renderer->GetHeight())))
			.setClearValueCount(2) // TODO: really 2?
			.setPClearValues(&clear_value);

	command_buffer.begin(vk::CommandBufferBeginInfo());

	auto viewport = vk::Viewport(0, 0, this->renderer->GetWidth(), this->renderer->GetHeight(), 0.0f, 1.0f);
	command_buffer.setViewport(0, 1, (const vk::Viewport *)&viewport);

	auto scissor = vk::Rect2D(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(this->renderer->GetWidth(), this->renderer->GetHeight())));
	command_buffer.setScissor(0, 1, (const vk::Rect2D *)&scissor);

	// TODO command_buffer.setDepthBias()

	command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

	renderer->RecordRenderables(command_buffer,
			Material::DefaultRenderMode::Shadow,
			this->renderer->GetMaterialPipelineManager(),
			descriptor_set);

	command_buffer.endRenderPass();

	command_buffer.end();

	return command_buffer;
}

vk::ImageView SpotLightShadow::GetImageView()
{
	if(resolve_image_view)
		return resolve_image_view;
	if(shadow_image_view)
		return shadow_image_view;
	return depth_image_view;
}
