
#ifndef VULKAN_MATERIAL_INSTANCE_H
#define VULKAN_MATERIAL_INSTANCE_H

#include <map>

#include "texture.h"
#include "material/material.h"

namespace engine
{

class Engine;

class MaterialInstance
{
	public:
		typedef unsigned int TextureSlot;
		static const TextureSlot texture_slot_base_color = 0;

	private:
		Material * const material;

		std::map<TextureSlot, Texture> textures;

		vk::DescriptorPool descriptor_pool;
		vk::DescriptorSet descriptor_set;

		void CreateDescriptorSet();

	public:
		MaterialInstance(Material *material, vk::DescriptorPool descriptor_pool);
		~MaterialInstance();

		void WriteDescriptorSet();

		vk::DescriptorSet GetDescriptorSet() const 				{ return descriptor_set; }

		void SetTexture(TextureSlot slot, Texture texture);
		Texture *GetTexture(TextureSlot slot);
};

}

#endif //VULKAN_MATERIAL_H
