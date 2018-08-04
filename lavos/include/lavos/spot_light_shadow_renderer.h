
#ifndef LAVOS_SPOT_LIGHT_SHADOW_RENDERER_H
#define LAVOS_SPOT_LIGHT_SHADOW_RENDERER_H

#include "sub_renderer.h"
#include "material_pipeline_manager.h"

#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace lavos
{

class Engine;

class SpotLightShadowRenderer : public SubRenderer
{
	private:
		std::uint32_t width;
		std::uint32_t height;
		vk::Format format;

		vk::RenderPass render_pass;

		void CreateRenderPass();

	public:
		SpotLightShadowRenderer(lavos::Engine *engine, std::uint32_t width, std::uint32_t height);
		~SpotLightShadowRenderer() override;

		std::uint32_t GetWidth() const			{ return width; }
		std::uint32_t GetHeight() const			{ return height; }
		vk::Format GetFormat() const			{ return format; }
		vk::RenderPass GetRenderPass() const	{ return render_pass; }

		void AddMaterial(Material *material) override;
		void RemoveMaterial(Material *material) override;
};

}

#endif //LAVOS_SPOT_LIGHT_SHADOW_RENDERER_H
