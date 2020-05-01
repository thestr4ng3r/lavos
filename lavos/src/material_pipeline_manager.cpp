
#include <lavos/material_pipeline_manager.h>

#include "lavos/material_pipeline_manager.h"
#include "lavos/renderer.h"

#include "../glsl/common_glsl_cpp.h"

using namespace lavos;

MaterialPipelineManager::MaterialPipelineManager(Engine *engine, const MaterialPipelineConfiguration &config)
	: engine(engine),
	config(config)
{
}

MaterialPipelineManager::~MaterialPipelineManager()
{
	DestroyAllMaterialPipelines();
}

void MaterialPipelineManager::SetConfiguration(const MaterialPipelineConfiguration &config)
{
	if(config == this->config)
		return;

	this->config = config;
	RecreateAllMaterialPipelines();
}

MaterialPipeline MaterialPipelineManager::CreateMaterialPipeline(Material *material, Material::RenderMode render_mode)
{
	auto device = engine->GetVkDevice();

	auto pipeline = MaterialPipeline(material);

	// pipeline layout

	std::vector<vk::DescriptorSetLayout> descriptor_set_layouts = {
		config.renderer_descriptor_set_layout
	};

	static_assert(DESCRIPTOR_SET_INDEX_COMMON == 0, "descriptor set index mismatch");
	pipeline.renderer_descriptor_set_index = DESCRIPTOR_SET_INDEX_COMMON;

	auto descriptor_set_layout_id = material->GetDescriptorSetId(render_mode);
	auto material_descriptor_set_layout = material->GetDescriptorSetLayout(descriptor_set_layout_id);
	if(material_descriptor_set_layout)
	{
		static_assert(DESCRIPTOR_SET_INDEX_MATERIAL == 1, "descriptor set index mismatch");
		descriptor_set_layouts.push_back(material_descriptor_set_layout->layout);
		pipeline.material_descriptor_set_index = DESCRIPTOR_SET_INDEX_MATERIAL;
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


	// pipeline

	auto shader_stages = material->GetShaderStageCreateInfos(render_mode);

	auto vertex_binding_descriptions = material->GetVertexInputBindingDescriptions();
	auto vertex_attribute_descriptions = material->GetVertexInputAttributeDescriptions();

	auto vertex_input_info = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptionCount(static_cast<uint32_t>(vertex_binding_descriptions.size()))
			.setPVertexBindingDescriptions(vertex_binding_descriptions.data())
			.setVertexAttributeDescriptionCount(static_cast<uint32_t>(vertex_attribute_descriptions.size()))
			.setPVertexAttributeDescriptions(vertex_attribute_descriptions.data());

	auto input_assembly_info = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(material->GetPrimitiveTopology());

	vk::Viewport viewport(0.0f, 0.0f, config.extent.width, config.extent.height, 0.0f, 1.0f);
	vk::Rect2D scissor({0, 0}, config.extent);

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

	auto pipeline_info = vk::GraphicsPipelineCreateInfo()
			.setStageCount(static_cast<uint32_t>(shader_stages.size()))
			.setPStages(shader_stages.data())
			.setPVertexInputState(&vertex_input_info)
			.setPInputAssemblyState(&input_assembly_info)
			.setPViewportState(&viewport_state_info)
			.setPRasterizationState(&rasterizer_info)
			.setPMultisampleState(&multisample_info)
			.setPDepthStencilState(&depth_stencil_info)
			.setPColorBlendState(&config.color_blend_state_info.Get())
			.setPDynamicState(nullptr)
			.setLayout(pipeline.pipeline_layout)
			.setRenderPass(config.render_pass)
			.setSubpass(0);


	pipeline.pipeline = device.createGraphicsPipeline(nullptr, pipeline_info);

	return pipeline;
}

void MaterialPipelineManager::DestroyMaterialPipeline(const MaterialPipeline &material_pipeline)
{
	auto &device = engine->GetVkDevice();
	device.destroyPipeline(material_pipeline.pipeline);
	device.destroyPipelineLayout(material_pipeline.pipeline_layout);
}

void MaterialPipelineManager::DestroyAllMaterialPipelines()
{
	for(auto &material_pipeline : material_pipelines)
		DestroyMaterialPipeline(material_pipeline);
	material_pipelines.clear();
}


void MaterialPipelineManager::AddMaterial(Material *material)
{
	for(auto pipeline : material_pipelines)
	{
		if(pipeline.material == material)
			return;
	}

	material_pipelines.push_back(CreateMaterialPipeline(material, config.render_mode));
}

void MaterialPipelineManager::RemoveMaterial(Material *material)
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

void MaterialPipelineManager::RecreateAllMaterialPipelines()
{
	for(auto &material_pipeline : material_pipelines)
	{
		auto material = material_pipeline.material;
		DestroyMaterialPipeline(material_pipeline);
		material_pipeline = CreateMaterialPipeline(material, config.render_mode);
	}
}

MaterialPipeline *MaterialPipelineManager::GetMaterialPipeline(Material *material)
{
	for(auto &p : material_pipelines)
	{
		if(p.material == material)
			return &p;
	}
	return nullptr;
}

