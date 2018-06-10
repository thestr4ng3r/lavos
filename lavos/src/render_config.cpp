
#include "lavos/render_config.h"

using namespace lavos;

RenderConfig RenderConfigBuilder::Build()
{
	RenderConfig config;
	config.material_render_modes = { Material::DefaultRenderMode::ColorForward };

	if(shadow_enabled)
		config.material_render_modes.push_back(Material::DefaultRenderMode::Shadow);

	return config;
}
