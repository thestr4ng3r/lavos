
#include "material/material.h"
#include "engine.h"

using namespace engine;

Material::Material(engine::Engine *engine)
	: engine(engine)
{
}

Material::~Material()
{
	engine->GetVkDevice().destroyDescriptorSetLayout(descriptor_set_layout);
}

vk::ShaderModule Material::CreateShaderModule(vk::Device device, const std::vector<char> &code)
{
	return device.createShaderModule(
		vk::ShaderModuleCreateInfo()
			.setCodeSize(code.size())
			.setPCode(reinterpret_cast<const uint32_t *>(code.data())));
}
