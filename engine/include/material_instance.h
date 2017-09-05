
#ifndef VULKAN_MATERIAL_INSTANCE_H
#define VULKAN_MATERIAL_INSTANCE_H

#include "image.h"

namespace engine
{

class Engine;

class MaterialInstance
{
	private:
		Material *material;

		Image texture_image;
		vk::ImageView texture_image_view;

		vk::DescriptorSet descriptor_set;

		void CreateDescriptorSet(vk::DescriptorPool descriptor_pool);

	public:
		MaterialInstance(Material *material, vk::DescriptorPool descriptor_pool, std::string texture_file);
		~MaterialInstance();

		//vk::ImageView GetImageView() const		{ return texture_image_view; }
		vk::DescriptorSet GetDescriptorSet() const 		{ return descriptor_set; }
};

}

#endif //VULKAN_MATERIAL_H
