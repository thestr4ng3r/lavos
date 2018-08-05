
#include "lavos/spot_light_shadow_renderer.h"
#include "lavos/engine.h"

using namespace lavos;

SpotLightShadowRenderer::SpotLightShadowRenderer(Engine *engine, std::uint32_t width, std::uint32_t height)
	: SubRenderer(engine),
	width(width),
	height(height)
{
	format = vk::Format::eD16Unorm;

	CreateRenderPass();
	CreateDescriptorSetLayout();

	material_pipeline_manager = new MaterialPipelineManager(engine, CreateMaterialPipelineConfiguration());
}

SpotLightShadowRenderer::~SpotLightShadowRenderer()
{
	const auto &device = engine->GetVkDevice();
	device.destroyDescriptorSetLayout(descriptor_set_layout);
	device.destroyRenderPass(render_pass);
}

MaterialPipelineConfiguration SpotLightShadowRenderer::CreateMaterialPipelineConfiguration()
{
	return MaterialPipelineConfiguration(
			vk::Extent2D(width, height),
			descriptor_set_layout,
			render_pass);
}

void SpotLightShadowRenderer::CreateRenderPass()
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

void SpotLightShadowRenderer::CreateDescriptorSetLayout()
{
	std::array<vk::DescriptorSetLayoutBinding, 1> bindings = {
			// matrix
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

void SpotLightShadowRenderer::AddMaterial(Material *material)
{
	material_pipeline_manager->AddMaterial(material);
}

void SpotLightShadowRenderer::RemoveMaterial(Material *material)
{
	material_pipeline_manager->RemoveMaterial(material);
}
