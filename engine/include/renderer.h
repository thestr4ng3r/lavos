
#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include "engine.h"
#include "material.h"

namespace engine
{


struct MatrixUniformBuffer
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};

class Renderer
{
	public:
		struct MaterialPipeline
		{
			Material *material;

			vk::PipelineLayout pipeline_layout;
			vk::Pipeline pipeline;

			MaterialPipeline(Material *material) :
				material(material) {}
		};

	private:
		Engine * const engine;

		vk::RenderPass render_pass;

		vk::Extent2D screen_extent;

		std::vector<MaterialPipeline> material_pipelines;

		vk::DescriptorPool descriptor_pool;

		vk::DescriptorSetLayout descriptor_set_layout;
		vk::DescriptorSet descriptor_set;

		engine::Buffer matrix_uniform_buffer;

		void CreateDescriptorPool();
		void CreateDescriptorSetLayout();
		void CreateDescriptorSet();

		void CreateMatrixUniformBuffer();

		MaterialPipeline CreateMaterialPipeline(Material *material);
		void DestroyMaterialPipeline(const MaterialPipeline &material_pipeline);

	public:
		Renderer(Engine *engine, vk::Extent2D screen_extent, vk::RenderPass render_pass);
		~Renderer();

		Engine *GetEngine() const 						{ return engine; }

		vk::DescriptorPool GetDescriptorPool() const 	{ return descriptor_pool; }

		void AddMaterial(Material *material);
		void RemoveMaterial(Material *material);

		void UpdateMatrixUniformBuffer();

		MaterialPipeline GetMaterialPipeline(int index)		{ return material_pipelines[index]; }
		vk::DescriptorSet GetDescriptorSet()			{ return descriptor_set; }
};

}

#endif //VULKAN_RENDERER_H
