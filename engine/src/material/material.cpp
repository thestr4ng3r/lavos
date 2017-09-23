
#include "material/material.h"
#include "engine.h"

#include "shader_load.h"

using namespace engine;

Material::Material(engine::Engine *engine)
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
