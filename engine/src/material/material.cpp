
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