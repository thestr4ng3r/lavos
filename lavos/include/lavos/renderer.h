
#ifndef LAVOS_RENDERER_H
#define LAVOS_RENDERER_H

#include <map>
#include <memory>

#include "component/camera_component.h"
#include "engine.h"
#include "material/material.h"
#include "mesh.h"
#include "scene.h"
#include "render_target.h"
#include "render_config.h"

namespace lavos
{

class SpotLightComponent;
class SpotLightShadow;

struct MatrixUniformBuffer
{
	glm::mat4 modelview;
	glm::mat4 projection;
};

static_assert(sizeof(MatrixUniformBuffer) == 128);


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
};

static_assert(sizeof(LightingUniformBufferFixed) == 48);
static_assert(sizeof(LightingUniformBufferSpotLight) == 32);


struct CameraUniformBuffer
{
	glm::vec3 position;
};

static_assert(sizeof(CameraUniformBuffer) == 12);


struct TransformPushConstant
{
	glm::mat4 transform;
};

static_assert(sizeof(TransformPushConstant) == 64);



class Renderer: public ColorRenderTarget::ChangedCallback
{
	public:
		struct MaterialPipeline
		{
			Material *material;

			vk::PipelineLayout pipeline_layout;
			vk::Pipeline pipeline;

			int renderer_descriptor_set_index;
			int material_descriptor_set_index;

			MaterialPipeline(Material *material) :
				material(material) {}
		};

	private:
		Engine * const engine;

		const RenderConfig config;

		Scene *scene = nullptr;
		CameraComponent *camera = nullptr;

		bool auto_set_camera_aspect = true;

		vk::CommandBuffer render_command_buffer;

		ColorRenderTarget *color_render_target;
		DepthRenderTarget *depth_render_target;

		std::vector<vk::Framebuffer> dst_framebuffers;


		vk::RenderPass render_pass;


		std::vector<MaterialPipeline> material_pipelines;

		vk::DescriptorPool descriptor_pool;

		vk::DescriptorSetLayout descriptor_set_layout;
		vk::DescriptorSet descriptor_set;

		lavos::Buffer *matrix_uniform_buffer;
		lavos::Buffer *lighting_uniform_buffer;
		lavos::Buffer *camera_uniform_buffer;

		void CreateFramebuffers();

		void CleanupFramebuffers();
		void CreateDescriptorPool();

		void CreateDescriptorSetLayout();
		void CreateDescriptorSet();

		size_t GetLightingUniformBufferSize();

		void CreateUniformBuffers();

		void RecreateAllMaterialPipelines();

		void CreateRenderPasses();
		void CleanupRenderPasses();

		void CreateRenderCommandBuffer();
		void CleanupRenderCommandBuffer();

		void RecordRenderCommandBuffer(vk::CommandBuffer command_buffer, vk::Framebuffer dst_framebuffer);

	protected:
		void RenderTargetChanged(RenderTarget *render_target) override;

	public:
		static const unsigned int max_spot_lights = 16;

		Renderer(Engine *engine, const RenderConfig &config, ColorRenderTarget *color_render_target, DepthRenderTarget *depth_render_target);
		~Renderer();

		Engine *GetEngine() const 							{ return engine; }

		vk::DescriptorPool GetDescriptorPool() const 		{ return descriptor_pool; }

		void SetScene(Scene *scene)							{ this->scene = scene; }
		void SetCamera(CameraComponent *camera)				{ this->camera = camera; }

		void AddMaterial(Material *material);
		void RemoveMaterial(Material *material);

		MaterialPipeline CreateMaterialPipeline(Material *material, Material::RenderMode render_mode);
		void DestroyMaterialPipeline(const MaterialPipeline &material_pipeline);

		void UpdateMatrixUniformBuffer();
		void UpdateCameraUniformBuffer();
		void UpdateLightingUniformBuffer();

		MaterialPipeline GetMaterialPipeline(int index)		{ return material_pipelines[index]; }
		vk::DescriptorSet GetDescriptorSet()				{ return descriptor_set; }

		vk::RenderPass GetRenderPass() const				{ return render_pass; }

		bool GetAutoSetCameraAspect() const 				{ return auto_set_camera_aspect; }
		void SetAutoSetCameraAspect(bool enabled)			{ auto_set_camera_aspect = enabled; }

		void DrawFrame(std::uint32_t image_index,
					   std::vector<vk::Semaphore> wait_semaphores,
					   std::vector<vk::PipelineStageFlags> wait_stages,
					   std::vector<vk::Semaphore> signal_semaphores);

		void DrawFrameRecord(vk::CommandBuffer command_buffer, vk::Framebuffer dst_framebuffer);
};

}

#endif //VULKAN_RENDERER_H
