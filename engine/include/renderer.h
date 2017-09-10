
#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include "engine.h"
#include "material.h"
#include "mesh.h"
#include "scene.h"

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

		Scene *scene;

		vk::CommandPool command_pool;
		std::vector<vk::CommandBuffer> command_buffers;

		vk::Format format;
		std::vector<vk::ImageView> dst_image_views;

		vk::Format depth_format;
		engine::Image depth_image;
		vk::ImageView depth_image_view;

		std::vector<vk::Framebuffer> dst_framebuffers;


		vk::RenderPass render_pass;

		vk::Extent2D screen_extent;

		std::vector<MaterialPipeline> material_pipelines;

		vk::DescriptorPool descriptor_pool;

		vk::DescriptorSetLayout descriptor_set_layout;
		vk::DescriptorSet descriptor_set;

		engine::Buffer matrix_uniform_buffer;

		void CreateCommandPool();
		void CleanupCommandPool();

		void CreateFramebuffers();
		void CleanupFramebuffers();

		void CreateDescriptorPool();
		void CreateDescriptorSetLayout();
		void CreateDescriptorSet();

		void CreateMatrixUniformBuffer();

		MaterialPipeline CreateMaterialPipeline(Material *material);
		void DestroyMaterialPipeline(const MaterialPipeline &material_pipeline);
		void RecreateAllMaterialPipelines();

		void CreateDepthResources();
		void CleanupDepthResources();

		void CreateRenderPasses();
		void CleanupRenderPasses();

		void CleanupCommandBuffers();

	public:
		Renderer(Engine *engine, vk::Extent2D screen_extent, vk::Format format, std::vector<vk::ImageView> dst_image_views);
		~Renderer();

		Engine *GetEngine() const 							{ return engine; }

		vk::DescriptorPool GetDescriptorPool() const 		{ return descriptor_pool; }

		void SetScene(Scene *scene)							{ this->scene = scene; }

		void AddMaterial(Material *material);
		void RemoveMaterial(Material *material);

		void UpdateMatrixUniformBuffer();

		void ResizeScreen(vk::Extent2D screen_extent, std::vector<vk::ImageView> dst_image_views);

		MaterialPipeline GetMaterialPipeline(int index)		{ return material_pipelines[index]; }
		vk::DescriptorSet GetDescriptorSet()				{ return descriptor_set; }

		vk::RenderPass GetRenderPass()						{ return render_pass; }
		vk::ImageView GetDepthImageView()					{ return depth_image_view; }

		void DrawFrame(std::uint32_t image_index,
					   std::vector<vk::Semaphore> wait_semaphores,
					   std::vector<vk::PipelineStageFlags> wait_stages,
					   std::vector<vk::Semaphore> signal_semaphores);


		void CreateCommandBuffers(); // TODO: make private
};

}

#endif //VULKAN_RENDERER_H
