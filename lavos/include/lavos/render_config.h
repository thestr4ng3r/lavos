
#ifndef LAVOS_RENDER_CONFIG_H
#define LAVOS_RENDER_CONFIG_H

#include "material/material.h"

namespace lavos
{

class RenderConfigBuilder;

class RenderConfig
{
	friend class RenderConfigBuilder;

	private:
		std::vector<Material::RenderMode> material_render_modes;

	public:
		const std::vector<Material::RenderMode> &GetMaterialRenderModes() const 	{ return material_render_modes; }
};

class RenderConfigBuilder
{
	private:
		bool shadow_enabled = false;

	public:
		RenderConfigBuilder &SetShadowEnabled(bool enabled)		{ shadow_enabled = enabled; return *this; }

		RenderConfig Build();
};

}

#endif //LAVOS_RENDER_CONFIG_H
