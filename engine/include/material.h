
#ifndef VULKAN_MATERIAL_H
#define VULKAN_MATERIAL_H

#include "image.h"

namespace engine
{

class Engine;

class Material
{
	private:
		Engine *engine;

		Image texture_image;
		vk::ImageView texture_image_view;

	public:
		Material(Engine *engine, std::string texture_file);
		~Material();

		vk::ImageView GetImageView() const	{ return texture_image_view; }
};

}

#endif //VULKAN_MATERIAL_H
