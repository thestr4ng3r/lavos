
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
	engine->GetVkDevice().destroyDescriptorSetLayout(descriptor_set_layout);
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
