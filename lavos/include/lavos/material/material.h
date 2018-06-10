
#ifndef LAVOS_MATERIAL_H
#define LAVOS_MATERIAL_H

#include <vulkan/vulkan.hpp>

#include <map>

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

		using DescriptorSetId = int;
		using InstanceDataId = int;

		using TextureSlot = unsigned int;
		using ParameterSlot = unsigned int;

		static const TextureSlot texture_slot_base_color = 0;
		static const TextureSlot texture_slot_normal = 1;

		static const ParameterSlot parameter_slot_base_color_factor = 0;

		struct DescriptorSetLayout
		{
			vk::DescriptorSetLayout layout;
			std::vector<vk::DescriptorPoolSize> pool_sizes;
		};

	private:
		std::map<RenderMode, DescriptorSetLayout> descriptor_set_layouts;

	protected:
		Engine *engine;

		void CreateDescriptorSetLayout(DescriptorSetId id, const std::vector<vk::DescriptorSetLayoutBinding> &bindings);

	public:
		Material(Engine *engine);
		virtual ~Material();

		Engine *GetEngine() const							 		{ return engine; }

		const DescriptorSetLayout *GetDescriptorSetLayout(DescriptorSetId id) const;

		virtual bool GetRenderModeSupport(RenderMode render_mode) const =0;

		virtual std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfos(RenderMode render_mode) const =0;

		virtual DescriptorSetId GetDescriptorSetId(RenderMode render_mode) const 					{ return render_mode; };
		virtual void WriteDescriptorSet(DescriptorSetId id, vk::DescriptorSet descriptor_set, MaterialInstance *instance) =0;

		virtual InstanceDataId GetInstanceDataId(RenderMode render_mode)							{ return render_mode; }
		virtual void *CreateInstanceData(InstanceDataId id)											{ return nullptr; }
		virtual void DestroyInstanceData(InstanceDataId id, void *data)								{}
		virtual void UpdateInstanceData(InstanceDataId id, void *data, MaterialInstance *instance) 	{}

		static vk::ShaderModule CreateShaderModule(vk::Device device, std::string shader);

		virtual Texture GetTextureDefaultImage() const 		{ return nullptr; };


		virtual vk::PrimitiveTopology GetPrimitiveTopology()	{ return vk::PrimitiveTopology::eTriangleList; }

		virtual std::vector<vk::VertexInputBindingDescription> GetVertexInputBindingDescriptions();
		virtual std::vector<vk::VertexInputAttributeDescription> GetVertexInputAttributeDescriptions();
};

}

#endif //VULKAN_MATERIAL_H
