
#ifndef VULKAN_MESH_APPLICATION_H
#define VULKAN_MESH_APPLICATION_H

#include <demo_application.h>
#include <vulkan/vulkan.hpp>
#include <engine.h>
#include <material.h>
#include <material_instance.h>

#include <glm/glm.hpp>

#include "demo_application.h"

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 uv;

	static vk::VertexInputBindingDescription GetBindingDescription()
	{
		return vk::VertexInputBindingDescription()
			.setBinding(0)
			.setStride(sizeof(Vertex))
			.setInputRate(vk::VertexInputRate::eVertex);
	}

	static std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescription()
	{
		return {
				vk::VertexInputAttributeDescription()
					.setBinding(0)
					.setLocation(0)
					.setFormat(vk::Format::eR32G32B32Sfloat)
					.setOffset(static_cast<uint32_t>(offsetof(Vertex, pos))),

				vk::VertexInputAttributeDescription()
					.setBinding(0)
					.setLocation(1)
					.setFormat(vk::Format::eR32G32B32Sfloat)
					.setOffset(static_cast<uint32_t>(offsetof(Vertex, color))),

				vk::VertexInputAttributeDescription()
					.setBinding(0)
					.setLocation(2)
					.setFormat(vk::Format::eR32G32Sfloat)
					.setOffset(static_cast<uint32_t>(offsetof(Vertex, uv))),
		};
	};
};

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

		vk::DescriptorPool descriptor_pool;
		vk::DescriptorSetLayout descriptor_set_layout;
		vk::DescriptorSet descriptor_set;

		const std::vector<Vertex> vertices = {
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

			{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
		};

		const std::vector<uint16_t> indices = {
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		};

		engine::Buffer vertex_buffer;
		engine::Buffer index_buffer;
		engine::Buffer matrix_uniform_buffer;

		engine::Material *material;
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

		void CreateDescriptorPool();
		void CreateDescriptorSetLayout();
		void CreateDescriptorSet();

		void CreateMaterial();

		void UpdateMatrixUniformBuffer();

		void DrawFrame(uint32_t image_index) override;
};

#endif
