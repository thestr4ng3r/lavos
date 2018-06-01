
#include "lavos/spot_light_shadow.h"
#include "lavos/component/spot_light_component.h"
#include "lavos/renderer.h"

using namespace lavos;

SpotLightShadow::SpotLightShadow(Engine *engine, SpotLightComponent *light, std::uint32_t width, std::uint32_t height)
		: engine(engine), light(light), width(width), height(height)
{
	format = vk::Format::eD16Unorm;
	min_filter = vk::Filter::eLinear;
	mag_filter = vk::Filter::eLinear;

	CreateImage();
	CreateRenderPass();
	CreateFramebuffer();
}

SpotLightShadow::~SpotLightShadow()
{
}

void SpotLightShadow::CreateImage()
{
	auto image_create_info = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setExtent(vk::Extent3D(width, height, 1))
			.setMipLevels(1)
			.setArrayLayers(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setFormat(format)
			.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);

	image = engine->CreateImage(image_create_info, VMA_MEMORY_USAGE_GPU_ONLY);

	auto image_view_create_info = vk::ImageViewCreateInfo()
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(format)
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

void SpotLightShadow::CreateRenderPass()
{
	auto attachment_desc = vk::AttachmentDescription()
			.setFormat(format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal);

	auto depth_reference = vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	auto subpass_desc = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(0)
			.setPDepthStencilAttachment(&depth_reference);

	std::array<vk::SubpassDependency, 2> dependencies;

	dependencies[0]
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
			.setDstStageMask(vk::PipelineStageFlagBits::eLateFragmentTests)
			.setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
			.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite)
			.setDependencyFlags(vk::DependencyFlagBits::eByRegion);

	dependencies[1]
			.setSrcSubpass(0).setDstSubpass(VK_SUBPASS_EXTERNAL)
			.setSrcStageMask(vk::PipelineStageFlagBits::eLateFragmentTests)
			.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
			.setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite)
			.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
			.setDependencyFlags(vk::DependencyFlagBits::eByRegion);

	auto create_info = vk::RenderPassCreateInfo()
			.setAttachmentCount(1)
			.setPAttachments(&attachment_desc)
			.setSubpassCount(1)
			.setPSubpasses(&subpass_desc)
			.setDependencyCount(dependencies.size())
			.setPDependencies(dependencies.data());

	render_pass = engine->GetVkDevice().createRenderPass(create_info);
}

void SpotLightShadow::CreateFramebuffer()
{
	auto create_info = vk::FramebufferCreateInfo()
			.setRenderPass(render_pass)
			.setAttachmentCount(1)
			.setPAttachments(&image_view)
			.setWidth(width)
			.setHeight(height)
			.setLayers(1);

	framebuffer = engine->GetVkDevice().createFramebuffer(create_info);
}

void SpotLightShadow::BuildCommandBuffer(Renderer *renderer)
{
	const vk::Device &device = engine->GetVkDevice();

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
			.setRenderPass(render_pass)
			.setFramebuffer(framebuffer)
			.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(width, height)))
			.setClearValueCount(2) // TODO: really 2?
			.setPClearValues(&clear_value);

	command_buffer.begin(vk::CommandBufferBeginInfo());

	auto viewport = vk::Viewport(0, 0, width, height, 0.0f, 1.0f);
	command_buffer.setViewport(0, 1, &viewport);

	auto scissor = vk::Rect2D(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(width, height)));
	command_buffer.setScissor(0, 1, &scissor);

	// TODO command_buffer.setDepthBias()

	command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
	
	// TODO: render stuff
	
	command_buffer.endRenderPass();
	
	command_buffer.end();
}
