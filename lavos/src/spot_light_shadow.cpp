
#include "lavos/spot_light_shadow.h"
#include "lavos/component/spot_light_component.h"
#include "lavos/renderer.h"
#include "lavos/spot_light_shadow_renderer.h"

using namespace lavos;

SpotLightShadow::SpotLightShadow(Engine *engine, SpotLightComponent *light, SpotLightShadowRenderer *renderer)
		: engine(engine), light(light), renderer(renderer)
{
	min_filter = vk::Filter::eLinear;
	mag_filter = vk::Filter::eLinear;

	CreateImage();
	CreateFramebuffer();
	CreateUniformBuffer();
	CreateDescriptorPool();
	CreateDescriptorSet();
}

SpotLightShadow::~SpotLightShadow()
{
	engine->GetVkDevice().free(descriptor_pool, descriptor_set);
	engine->GetVkDevice().destroy(descriptor_pool);
	delete matrix_uniform_buffer;
}

void SpotLightShadow::CreateImage()
{
	auto image_create_info = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setExtent(vk::Extent3D(renderer->GetWidth(), renderer->GetHeight(), 1))
			.setMipLevels(1)
			.setArrayLayers(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setFormat(renderer->GetFormat())
			.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);

	image = engine->CreateImage(image_create_info, VMA_MEMORY_USAGE_GPU_ONLY);

	auto image_view_create_info = vk::ImageViewCreateInfo()
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(renderer->GetFormat())
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1))
			.setImage(image.image);

	image_view = engine->GetVkDevice().createImageView(image_view_create_info);

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
}

void SpotLightShadow::CreateFramebuffer()
{
	auto create_info = vk::FramebufferCreateInfo()
			.setRenderPass(renderer->GetRenderPass())
			.setAttachmentCount(1)
			.setPAttachments(&image_view)
			.setWidth(renderer->GetWidth())
			.setHeight(renderer->GetHeight())
			.setLayers(1);

	framebuffer = engine->GetVkDevice().createFramebuffer(create_info);
}

void SpotLightShadow::CreateUniformBuffer()
{
	matrix_uniform_buffer = engine->CreateBuffer(sizeof(MatrixUniformBuffer),
												 vk::BufferUsageFlagBits::eUniformBuffer,
												 VMA_MEMORY_USAGE_CPU_ONLY);
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
}

void SpotLightShadow::CreateDescriptorSet()
{
	vk::DescriptorSetLayout layouts[] = { renderer->GetDescriptorSetLayout() };

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
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setPBufferInfo(&matrix_buffer_info);

	engine->GetVkDevice().updateDescriptorSets(matrix_buffer_write, nullptr);
}

void SpotLightShadow::UpdateMatrixUniformBuffer()
{
	MatrixUniformBuffer matrix_ubo;
	matrix_ubo.modelview = light->GetModelViewMatrix();
	matrix_ubo.projection = light->GetProjectionMatrix(0.1f, 100.0f); // TODO: make configurable
	matrix_ubo.projection[1][1] *= -1.0f;

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
	}

	if(!semaphore)
	{
		semaphore = device.createSemaphore(vk::SemaphoreCreateInfo());
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
	command_buffer.setViewport(0, 1, &viewport);

	auto scissor = vk::Rect2D(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(this->renderer->GetWidth(), this->renderer->GetHeight())));
	command_buffer.setScissor(0, 1, &scissor);

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
