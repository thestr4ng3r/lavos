
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

		vk::DescriptorSetLayout descriptor_set_layout; // TODO: scope of this could be higher (common for all SpotLightShadowRenderers)
		vk::RenderPass render_pass; // TODO: scope of this could be higher (depends only on format)

		MaterialPipelineManager *material_pipeline_manager;

		MaterialPipelineConfiguration CreateMaterialPipelineConfiguration();

		void CreateRenderPass();
		void CreateDescriptorSetLayout();

	public:
		SpotLightShadowRenderer(lavos::Engine *engine, std::uint32_t width, std::uint32_t height);
		~SpotLightShadowRenderer() override;

		std::uint32_t GetWidth() const							{ return width; }
		std::uint32_t GetHeight() const							{ return height; }
		vk::Format GetFormat() const							{ return format; }
		vk::RenderPass GetRenderPass() const					{ return render_pass; }
		vk::DescriptorSetLayout GetDescriptorSetLayout() const	{ return descriptor_set_layout; }

		MaterialPipelineManager *GetMaterialPipelineManager() const { return material_pipeline_manager; }

		void AddMaterial(Material *material) override;
		void RemoveMaterial(Material *material) override;
};

}

#endif //LAVOS_SPOT_LIGHT_SHADOW_RENDERER_H
