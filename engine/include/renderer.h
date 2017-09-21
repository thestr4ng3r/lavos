
#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include <component/camera_component.h>
#include "engine.h"
#include "material/material.h"
#include "mesh.h"
#include "scene.h"

namespace engine
{


struct MatrixUniformBuffer
{
	glm::mat4 modelview;
	glm::mat4 projection;
};

struct alignas(sizeof(float)) LightingUniformBuffer
{
	glm::vec3 ambient_intensity;
	std::uint32_t directional_light_enabled;
	glm::vec3 directional_light_dir;
	std::uint8_t unused_2[4];
	glm::vec3 directional_light_intensity;
};

struct TransformPushConstant
{
	glm::mat4 transform;
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

		Scene *scene = nullptr;
		CameraComponent *camera = nullptr;

		bool auto_set_camera_aspect = true;

		vk::CommandPool render_command_pool;
		vk::CommandBuffer render_command_buffer;

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
		engine::Buffer lighting_uniform_buffer;

		void CreateRenderCommandPool();
		void CleanupRenderCommandPool();

		void CreateFramebuffers();
		void CleanupFramebuffers();

		void CreateDescriptorPool();
		void CreateDescriptorSetLayout();
		void CreateDescriptorSet();

		void CreateUniformBuffers();

		MaterialPipeline CreateMaterialPipeline(Material *material);
		void DestroyMaterialPipeline(const MaterialPipeline &material_pipeline);
		void RecreateAllMaterialPipelines();

		void CreateDepthResources();
		void CleanupDepthResources();

		void CreateRenderPasses();
		void CleanupRenderPasses();

		void CreateRenderCommandBuffer();
		void CleanupRenderCommandBuffer();

		void RecordRenderCommandBuffer(vk::Framebuffer dst_framebuffer);

	public:
		Renderer(Engine *engine, vk::Extent2D screen_extent, vk::Format format, std::vector<vk::ImageView> dst_image_views);
		~Renderer();

		Engine *GetEngine() const 							{ return engine; }

		vk::DescriptorPool GetDescriptorPool() const 		{ return descriptor_pool; }

		void SetScene(Scene *scene)							{ this->scene = scene; }
		void SetCamera(CameraComponent *camera)				{ this->camera = camera; }

		void AddMaterial(Material *material);
		void RemoveMaterial(Material *material);

		void UpdateMatrixUniformBuffer();
		void UpdateLightingUniformBuffer();

		void ResizeScreen(vk::Extent2D screen_extent, std::vector<vk::ImageView> dst_image_views);

		MaterialPipeline GetMaterialPipeline(int index)		{ return material_pipelines[index]; }
		vk::DescriptorSet GetDescriptorSet()				{ return descriptor_set; }

		vk::RenderPass GetRenderPass() const				{ return render_pass; }
		vk::ImageView GetDepthImageView()					{ return depth_image_view; }

		bool GetAutoSetCameraAspect() const 				{ return auto_set_camera_aspect; }
		void SetAutoSetCameraAspect(bool enabled)			{ auto_set_camera_aspect = enabled; }

		void DrawFrame(std::uint32_t image_index,
					   std::vector<vk::Semaphore> wait_semaphores,
					   std::vector<vk::PipelineStageFlags> wait_stages,
					   std::vector<vk::Semaphore> signal_semaphores);
};

}

#endif //VULKAN_RENDERER_H
