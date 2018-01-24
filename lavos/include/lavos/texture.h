
#ifndef VULKAN_TEXTURE_H
#define VULKAN_TEXTURE_H

#include "image.h"

namespace lavos
{

class Engine;

class Texture
{
	public:
		Image image;
		vk::ImageView image_view;
		vk::Sampler sampler;

		Texture(std::nullptr_t = nullptr)
			: image(), image_view(nullptr), sampler(nullptr) {}

		Texture(Image image, vk::ImageView image_view, vk::Sampler sampler)
			: image(image), image_view(image_view), sampler(sampler) {}


		static Texture CreateColor(Engine *engine, vk::Format format, glm::vec4 color);


		bool operator==(Texture const &rhs) const
		{
			return image == rhs.image
				   && image_view == rhs.image_view
				   && sampler == rhs.sampler;
		}

		bool operator!=(Texture const &rhs) const
		{
			return image != rhs.image
				   || image_view != rhs.image_view
				   || sampler != rhs.sampler;
		}
};

}

#endif //VULKAN_TEXTURE_H
