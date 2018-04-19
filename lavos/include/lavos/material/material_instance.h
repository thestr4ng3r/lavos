
#ifndef LAVOS_MATERIAL_INSTANCE_H
#define LAVOS_MATERIAL_INSTANCE_H

#include <map>

#include "../texture.h"
#include "../buffer.h"

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

		typedef unsigned int TextureSlot;
		typedef unsigned int ParameterSlot;

	private:
		Material * const material;

		std::map<TextureSlot, Texture> textures;
		std::map<ParameterSlot, Parameter> parameters;

		vk::DescriptorPool descriptor_pool;
		vk::DescriptorSet descriptor_set;

		void *instance_data;

		void CreateDescriptorSet();
		void CreateUniformBuffer();

		void WriteDescriptorSet();
		void WriteUniformBuffer();

	public:
		MaterialInstance(Material *material, vk::DescriptorPool descriptor_pool);
		~MaterialInstance();

		void WriteAllData();

		vk::DescriptorSet GetDescriptorSet() const 				{ return descriptor_set; }
		void *GetInstanceData() const 							{ return instance_data; }

		void SetTexture(TextureSlot slot, Texture texture);
		Texture *GetTexture(TextureSlot slot);

		void SetParameter(ParameterSlot slot, Parameter parameter);
		Parameter *GetParameter(ParameterSlot slot);
		float GetParameter(ParameterSlot slot, float default_value);
		glm::vec2 GetParameter(ParameterSlot slot, glm::vec2 default_value);
		glm::vec3 GetParameter(ParameterSlot slot, glm::vec3 default_value);
		glm::vec4 GetParameter(ParameterSlot slot, glm::vec4 default_value);
};

}

#endif //VULKAN_MATERIAL_H
