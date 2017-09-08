
#ifndef VULKAN_MATERIAL_INSTANCE_H
#define VULKAN_MATERIAL_INSTANCE_H

#include "texture.h"
#include "material.h"

namespace engine
{

class Engine;

class MaterialInstance
{
	private:
		Material * const material;

		Texture texture;

		vk::DescriptorSet descriptor_set;

		void CreateDescriptorSet(vk::DescriptorPool descriptor_pool);

	public:
		MaterialInstance(Material *material, vk::DescriptorPool descriptor_pool);
		~MaterialInstance();

		void WriteDescriptorSet();

		vk::DescriptorSet GetDescriptorSet() const 		{ return descriptor_set; }

		void SetTexture(Texture texture)				{ this->texture = texture; };
};

}

#endif //VULKAN_MATERIAL_H
