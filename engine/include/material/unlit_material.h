
#ifndef VULKAN_UNLIT_MATERIAL_H
#define VULKAN_UNLIT_MATERIAL_H

#include "material.h"

namespace engine
{

class UnlitMaterial: public Material
{
	private:
		void CreateDescriptorSetLayout();

	public:
		UnlitMaterial(Engine *engine);
		~UnlitMaterial();

		virtual std::vector<vk::DescriptorPoolSize> GetDescriptorPoolSizes() const override;
};

}

#endif //VULKAN_UNLIT_MATERIAL_H
