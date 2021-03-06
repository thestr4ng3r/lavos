
#ifndef LAVOS_RENDERER_H
#define LAVOS_RENDERER_H

#include <map>
#include <memory>

#include "lavos/component/camera.h"
#include "engine.h"
#include "material/material.h"
#include "mesh.h"
#include "scene.h"
#include "render_target.h"
#include "render_config.h"
#include "material_pipeline_manager.h"

namespace lavos
{

class SpotLight;
class SpotLightShadow;
class SubRenderer;
class LightCollection;

struct MatrixUniformBuffer
{
	glm::mat4 modelview;
	glm::mat4 projection;
};

static_assert(sizeof(MatrixUniformBuffer) == 128, "MatrixUniformBuffer memory layout");


struct LightingUniformBufferFixed
{
	glm::vec3 ambient_intensity;
	std::uint32_t directional_light_enabled;
	glm::vec3 directional_light_dir;
	std::uint8_t unused_2[4];
	glm::vec3 directional_light_intensity;
	std::uint32_t spot_lights_count;
};

struct LightingUniformBufferSpotLight
{
	glm::vec3 position;
	float angle_cos;
	glm::vec3 direction;
	float unused;
	glm::mat4 shadow_mvp_matrix;
};

static_assert(sizeof(LightingUniformBufferFixed) == 48, "LightingUniformBufferFixed memory layout");
static_assert(sizeof(LightingUniformBufferSpotLight) == 96, "LightingUniformBufferSpotLight memory layout");


struct CameraUniformBuffer
{
	glm::vec3 position;
};

static_assert(sizeof(CameraUniformBuffer) == 12, "CameraUniformBuffer memory layout");


struct TransformPushConstant
{
	glm::mat4 transform;
};

static_assert(sizeof(TransformPushConstant) == 64, "TransformPushConstant memory layout");



class Renderer: public ColorRenderTarget::ChangedCallback
{
	friend class MaterialPipelineManager;

	private:
		Engine * const engine;

		const RenderConfig config;

		Scene *scene = nullptr;
		Camera *camera = nullptr;

		bool auto_set_camera_aspect = true;

		vk::CommandBuffer render_command_buffer;

		ColorRenderTarget *color_render_target;
		DepthRenderTarget *depth_render_target;

		std::vector<vk::Framebuffer> dst_framebuffers;

		MaterialPipelineManager *material_pipeline_manager;

		Texture spot_light_shadow_default;

		vk::RenderPass render_pass;

		vk::DescriptorPool descriptor_pool;

		vk::DescriptorSetLayout descriptor_set_layout;
		vk::DescriptorSet descriptor_set;

		lavos::Buffer *matrix_uniform_buffer;
		lavos::Buffer *lighting_uniform_buffer;
		lavos::Buffer *camera_uniform_buffer;

		std::vector<Material *> materials;
		std::vector<SubRenderer *> sub_renderers;


		MaterialPipelineConfiguration CreateMaterialPipelineConfiguration();

		void CreateFramebuffers();

		void CleanupFramebuffers();
		void CreateDescriptorPool();

		void CreateDescriptorSetLayout();
		void CreateDescriptorSet();

		size_t GetLightingUniformBufferSize();

		void CreateUniformBuffers();

		void CreateRenderPasses();
		void CleanupRenderPasses();

		void CreateRenderCommandBuffer();
		void CleanupRenderCommandBuffer();

	protected:
		void RenderTargetChanged(RenderTarget *render_target) override;

	public:
		static const unsigned int max_spot_lights = 16;

		Renderer(Engine *engine, const RenderConfig &config, ColorRenderTarget *color_render_target, DepthRenderTarget *depth_render_target);
		~Renderer();

		Engine *GetEngine() const 							{ return engine; }

		//vk::DescriptorPool GetDescriptorPool() const 		{ return descriptor_pool; }

		void SetScene(Scene *scene)							{ this->scene = scene; }
		void SetCamera(Camera *camera)				{ this->camera = camera; }

		/**
		 * Adds a SubRenderer to the Renderer.
		 * The Renderer takes ownership of sub_renderer.
		 */
		void AddSubRenderer(SubRenderer *sub_renderer);

		/**
		 * Removes a SubRenderer from the Renderer.
		 * The ownership of sub_renderer is removed from the Renderer.
		 */
		void RemoveSubRenderer(SubRenderer *sub_renderer);

		void AddMaterial(Material *material);
		void RemoveMaterial(Material *material);

		void UpdateMatrixUniformBuffer();
		void UpdateCameraUniformBuffer();
		void UpdateLightingUniformBuffer(LightCollection *light_collection);
		void UpdateShadowDescriptors(LightCollection *light_collection);

		//MaterialPipeline GetMaterialPipeline(int index)		{ return material_pipelines[index]; }

		vk::RenderPass GetRenderPass() const				{ return render_pass; }

		bool GetAutoSetCameraAspect() const 				{ return auto_set_camera_aspect; }
		void SetAutoSetCameraAspect(bool enabled)			{ auto_set_camera_aspect = enabled; }

		void DrawFrame(std::uint32_t image_index,
					   std::vector<vk::Semaphore> wait_semaphores,
					   std::vector<vk::PipelineStageFlags> wait_stages,
					   std::vector<vk::Semaphore> signal_semaphores);

		void DrawFrameRecord(vk::CommandBuffer command_buffer, vk::Framebuffer dst_framebuffer);
		void RecordRenderables(vk::CommandBuffer command_buffer,
							   Material::RenderMode render_mode,
							   MaterialPipelineManager *material_pipeline_manager,
							   vk::DescriptorSet renderer_descriptor_set);
};

}

#endif //VULKAN_RENDERER_H
