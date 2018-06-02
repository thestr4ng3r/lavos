
#ifndef LAVOS_MATERIAL_INSTANCE_H
#define LAVOS_MATERIAL_INSTANCE_H

#include <map>
#include <unordered_map>

#include "../texture.h"
#include "../buffer.h"
#include "material.h"

namespace lavos
{

class Engine;
class Material;

class MaterialInstance
{
	public:
		struct Parameter
		{
			union
			{
				float f;
				glm::vec2 v2;
				glm::vec3 v3;
				glm::vec4 v4 = glm::vec4(0.0f);
			};

			Parameter(float value)				{ f = value; }
			Parameter(const glm::vec2 &value)	{ v2 = value; }
			Parameter(const glm::vec3 &value)	{ v3 = value; }
			Parameter(const glm::vec4 &value)	{ v4 = value; }
		};

	private:
		Material * const material;

		std::map<Material::TextureSlot, Texture> textures;
		std::map<Material::ParameterSlot, Parameter> parameters;

		vk::DescriptorPool descriptor_pool;

		std::unordered_map<Material::RenderMode, vk::DescriptorSet> descriptor_sets;
		std::unordered_map<Material::RenderMode, void *> instance_data;

		void CreateDescriptorSet(Material::RenderMode render_mode);
		void CreateInstanceData(Material::RenderMode render_mode);

		void WriteDescriptorSet(Material::RenderMode render_mode);
		void WriteInstanceData(Material::RenderMode render_mode);

	public:
		MaterialInstance(Material *material, vk::DescriptorPool descriptor_pool);
		~MaterialInstance();

		void WriteAllData();

		vk::DescriptorSet GetDescriptorSet(Material::RenderMode render_mode) const	{ auto it = descriptor_sets.find(render_mode); return it == descriptor_sets.end() ? vk::DescriptorSet() : it->second; }
		void *GetInstanceData(Material::RenderMode render_mode) const				{ auto it = instance_data.find(render_mode); return it == instance_data.end() ? nullptr : it->second; }

		void SetTexture(Material::TextureSlot slot, Texture texture);
		Texture *GetTexture(Material::TextureSlot slot);

		void SetParameter(Material::ParameterSlot slot, Parameter parameter);
		Parameter *GetParameter(Material::ParameterSlot slot);
		float GetParameter(Material::ParameterSlot slot, float default_value);
		glm::vec2 GetParameter(Material::ParameterSlot slot, glm::vec2 default_value);
		glm::vec3 GetParameter(Material::ParameterSlot slot, glm::vec3 default_value);
		glm::vec4 GetParameter(Material::ParameterSlot slot, glm::vec4 default_value);
};

}

#endif //VULKAN_MATERIAL_H
