
#include "lavos/material/material.h"
#include "lavos/engine.h"
#include "lavos/vertex.h"

#include "lavos/shader_load.h"

using namespace lavos;

Material::Material(lavos::Engine *engine)
	: engine(engine)
{
}

Material::~Material()
{
	for(auto &it : descriptor_set_layouts)
		engine->GetVkDevice().destroyDescriptorSetLayout(it.second.layout);
}

void Material::CreateDescriptorSetLayout(DescriptorSetId id,
										 const std::vector<vk::DescriptorSetLayoutBinding> &bindings)
{
	assert(descriptor_set_layouts.find(id) == descriptor_set_layouts.end());

	auto create_info = vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(static_cast<uint32_t>(bindings.size()))
			.setPBindings(bindings.data());

	DescriptorSetLayout layout;
	layout.layout = engine->GetVkDevice().createDescriptorSetLayout(create_info);

	std::map<vk::DescriptorType, uint32_t> descriptor_counts;
	for(auto binding : bindings)
	{
		if(descriptor_counts.find(binding.descriptorType) == descriptor_counts.end())
			descriptor_counts[binding.descriptorType] = 1;
		else
			descriptor_counts[binding.descriptorType]++;
	}

	layout.pool_sizes.resize(descriptor_counts.size());
	int i=0;
	for(auto it : descriptor_counts)
		layout.pool_sizes[i++] = vk::DescriptorPoolSize(it.first, it.second);

	descriptor_set_layouts[id] = layout;
}

const Material::DescriptorSetLayout *Material::GetDescriptorSetLayout(DescriptorSetId id) const
{
	auto it = descriptor_set_layouts.find(id);
	if(it == descriptor_set_layouts.end())
		return nullptr;
	return &it->second;
}

vk::ShaderModule Material::CreateShaderModule(vk::Device device, std::string shader)
{
	size_t size;
	const uint32_t *code = GetSPIRVShader(shader, &size);

	return device.createShaderModule(
		vk::ShaderModuleCreateInfo()
			.setCodeSize(size)
			.setPCode(code));
}

std::vector<vk::VertexInputBindingDescription> Material::GetVertexInputBindingDescriptions()
{
	return { Vertex::GetBindingDescription() };
}

std::vector<vk::VertexInputAttributeDescription> Material::GetVertexInputAttributeDescriptions()
{
	return Vertex::GetAttributeDescription();
}

Material::UBOInstanceData::UBOInstanceData(lavos::Buffer *uniform_buffer)
		: uniform_buffer(uniform_buffer)
{
}

Material::UBOInstanceData::~UBOInstanceData()
{
	delete uniform_buffer;
}
