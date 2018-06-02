
#ifndef LAVOS_MATERIAL_H
#define LAVOS_MATERIAL_H

#include <vulkan/vulkan.hpp>

#include "../texture.h"

namespace lavos
{

class Engine;
class MaterialInstance;

/**
 * Describes one type of Material with shaders and some Material-specific settings
 * used to build a Vulkan Pipeline.
 *
 * This is not a material in the sense of a collection of settings and textures, but only
 * the type of material, e.g. Phong, PBR, etc.
 * The actual settings are defined by a MaterialInstance.
 */
class Material
{
	protected:
		Engine *engine;

		vk::DescriptorSetLayout descriptor_set_layout;

	public:
		/**
		 * Mode in which a Material can be rendered, e.g. default forward or shadow.
		 * See DefaultRenderMode for builtin values.
		 */
		using RenderMode = int;

		/**
		 * Builtin values for RenderMode.
		 * Custom modes can start from DefaultRenderMode::User0.
		 */
		enum DefaultRenderMode : RenderMode {
			ColorForward = 0,
			Shadow,
			User0
		};

		using TextureSlot = unsigned int;
		using ParameterSlot = unsigned int;

		static const TextureSlot texture_slot_base_color = 0;
		static const TextureSlot texture_slot_normal = 1;

		static const ParameterSlot parameter_slot_base_color_factor = 0;


		Material(Engine *engine);
		virtual ~Material();

		Engine *GetEngine() const							 		{ return engine; }

		vk::DescriptorSetLayout GetDescriptorSetLayout() const		{ return descriptor_set_layout; }

		virtual bool GetRenderModeSupport(RenderMode render_mode) const =0;

		virtual std::vector<vk::DescriptorPoolSize> GetDescriptorPoolSizes(RenderMode render_mode) const =0;
		virtual std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfos(RenderMode render_mode) const =0;

		virtual void WriteDescriptorSet(RenderMode render_mode, vk::DescriptorSet descriptor_set, MaterialInstance *instance) =0;

		virtual void *CreateInstanceData(RenderMode render_mode)											{ return nullptr; }
		virtual void DestroyInstanceData(RenderMode render_mode, void *data)								{}
		virtual void UpdateInstanceData(RenderMode render_mode, void *data, MaterialInstance *instance) 	{}

		static vk::ShaderModule CreateShaderModule(vk::Device device, std::string shader);

		virtual Texture GetTextureDefaultImage() const 		{ return nullptr; };


		virtual vk::PrimitiveTopology GetPrimitiveTopology()	{ return vk::PrimitiveTopology::eTriangleList; }

		virtual std::vector<vk::VertexInputBindingDescription> GetVertexInputBindingDescriptions();
		virtual std::vector<vk::VertexInputAttributeDescription> GetVertexInputAttributeDescriptions();
};

}

#endif //VULKAN_MATERIAL_H
