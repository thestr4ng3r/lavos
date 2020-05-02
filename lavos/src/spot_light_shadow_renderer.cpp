
#include "lavos/spot_light_shadow_renderer.h"
#include "lavos/engine.h"

#include "../glsl/common_glsl_cpp.h"

using namespace lavos;

SpotLightShadowRenderer::SpotLightShadowRenderer(Engine *engine, std::uint32_t width, std::uint32_t height)
	: SubRenderer(engine),
	width(width),
	height(height)
{
	depth_format = vk::Format::eD16Unorm;
#if SHADOW_MSM
	shadow_format = vk::Format::eR32G32B32A32Sfloat;
#else
	shadow_format = vk::Format::eUndefined;
#endif

	CreateRenderPass();
	CreateDescriptorSetLayout();

	material_pipeline_manager = new MaterialPipelineManager(engine, CreateMaterialPipelineConfiguration());
}

SpotLightShadowRenderer::~SpotLightShadowRenderer()
{
	delete material_pipeline_manager;
	const auto &device = engine->GetVkDevice();
	device.destroyDescriptorSetLayout(descriptor_set_layout);
	device.destroyRenderPass(render_pass);
}

MaterialPipelineConfiguration SpotLightShadowRenderer::CreateMaterialPipelineConfiguration()
{
	auto color_blend_state = vk_util::PipelineColorBlendStateCreateInfo();

	if(shadow_format != vk::Format::eUndefined)
	{
		auto color_blend_attachment = vk::PipelineColorBlendAttachmentState()
				.setColorWriteMask(vk::ColorComponentFlagBits::eR
								   | vk::ColorComponentFlagBits::eG
								   | vk::ColorComponentFlagBits::eB
								   | vk::ColorComponentFlagBits::eA)
				.setBlendEnable(VK_FALSE);

		color_blend_state.SetAttachments({ color_blend_attachment });
	}

	return MaterialPipelineConfiguration(
			vk::Extent2D(width, height),
			descriptor_set_layout,
			render_pass,
			Material::DefaultRenderMode::Shadow,
			color_blend_state);
}

void SpotLightShadowRenderer::CreateRenderPass()
{
	// whether we use an additional texture instead of just the depth buffer
	bool have_shadow_tex = shadow_format != vk::Format::eUndefined;

	std::array<vk::AttachmentDescription, 2> attachments;

	attachments[0] = vk::AttachmentDescription()
			.setFormat(depth_format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			// if we have a dedicated shadow tex, we will use this instead of the depth buffer
			.setFinalLayout(have_shadow_tex ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eShaderReadOnlyOptimal);

	auto depth_reference = vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	auto subpass_desc = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setPDepthStencilAttachment(&depth_reference);

	vk::AttachmentReference shadow_reference;
	if(have_shadow_tex)
	{
		attachments[1] = vk::AttachmentDescription()
				.setFormat(shadow_format)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(vk::AttachmentLoadOp::eClear)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(vk::ImageLayout::eUndefined)
				.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		shadow_reference = vk::AttachmentReference()
				.setAttachment(1)
				.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		subpass_desc
			.setColorAttachmentCount(1)
			.setPColorAttachments(&shadow_reference);
	}
	else
	{
		subpass_desc.setColorAttachmentCount(0);
	}

	std::array<vk::SubpassDependency, 2> dependencies;

	dependencies[0]
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
			.setDstStageMask(vk::PipelineStageFlagBits::eLateFragmentTests)
			.setSrcAccessMask(static_cast<vk::AccessFlags>(0))
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
			.setAttachmentCount(have_shadow_tex ? 2 : 1)
			.setPAttachments(attachments.data())
			.setSubpassCount(1)
			.setPSubpasses(&subpass_desc)
			.setDependencyCount(dependencies.size())
			.setPDependencies(dependencies.data());

	render_pass = engine->GetVkDevice().createRenderPass(create_info);
	vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), render_pass, "SpotLightShadowRenderer RenderPass");
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
	vk_util::SetDebugUtilsObjectName(engine->GetVkDevice(), descriptor_set_layout, "SpotLightShadowRenderer DescriptorSetLayout");
}

void SpotLightShadowRenderer::AddMaterial(Material *material)
{
	material_pipeline_manager->AddMaterial(material);
}

void SpotLightShadowRenderer::RemoveMaterial(Material *material)
{
	material_pipeline_manager->RemoveMaterial(material);
}
