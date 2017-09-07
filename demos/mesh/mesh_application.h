
#ifndef VULKAN_MESH_APPLICATION_H
#define VULKAN_MESH_APPLICATION_H

#include <demo_application.h>
#include <vulkan/vulkan.hpp>
#include <engine.h>
#include <vertex.h>
#include <material.h>
#include <material_instance.h>
#include <mesh.h>
#include <renderer.h>

#include <glm/glm.hpp>

#include "demo_application.h"

struct MatrixUniformBuffer
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};

class MeshApplication: public DemoApplication
{
    private:
		vk::Format depth_format;
		engine::Image depth_image;
		vk::ImageView depth_image_view;

		vk::RenderPass render_pass;

		vk::PipelineLayout pipeline_layout;
		vk::Pipeline pipeline;

		vk::CommandPool command_pool;
		std::vector<vk::CommandBuffer> command_buffers;

		vk::DescriptorSetLayout descriptor_set_layout;
		vk::DescriptorSet descriptor_set;

		engine::Renderer *renderer;

		engine::Mesh *mesh;

		engine::Buffer vertex_buffer;
		engine::Buffer index_buffer;
		engine::Buffer matrix_uniform_buffer;

		//engine::Material *material;
		engine::MaterialInstance *material_instance;

		void InitVulkan() override;

		void CreateDepthResources();

		void RecreateSwapchain() override;
		void CleanupSwapchain() override;

		void CleanupApplication() override;

		void CreateRenderPasses();

		vk::ShaderModule CreateShaderModule(const std::vector<char> &code);
		void CreatePipeline();

		void CreateFramebuffers();

		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateMatrixUniformBuffer();

		void CreateCommandPool();
		void CreateCommandBuffers();

		void CreateDescriptorSetLayout();
		void CreateDescriptorSet();

		void UpdateMatrixUniformBuffer();

		void DrawFrame(uint32_t image_index) override;
};

#endif
