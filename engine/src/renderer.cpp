
#include <chrono>
#include "glm_config.h"
#include <glm/gtc/matrix_transform.hpp>

#include "renderer.h"
#include "shader_load.h"
#include "vertex.h"

using namespace engine;

Renderer::Renderer(Engine *engine, vk::Extent2D screen_extent, vk::RenderPass render_pass)
	: engine(engine)
{
	this->screen_extent = screen_extent;
	this->render_pass = render_pass;

	CreateDescriptorPool();
	CreateDescriptorSetLayout();
	CreateMatrixUniformBuffer();
	CreateDescriptorSet();
}

Renderer::~Renderer()
{
	auto &device = engine->GetVkDevice();

	device.destroyDescriptorSetLayout(descriptor_set_layout);

	for(auto &material_pipeline : material_pipelines)
		DestroyMaterialPipeline(material_pipeline);

	device.destroyDescriptorPool(descriptor_pool);

	engine->DestroyBuffer(matrix_uniform_buffer);
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


