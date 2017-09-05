
#ifndef VULKAN_UNIFORMBUFFER_APPLICATION_H
#define VULKAN_UNIFORMBUFFER_APPLICATION_H

#include <demo_application.h>
#include <vulkan/vulkan.hpp>
#include <engine.h>

#include <glm/glm.hpp>

#include "demo_application.h"

struct Vertex
{
	glm::vec2 pos;
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
					.setFormat(vk::Format::eR32G32Sfloat)
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

class TextureApplication: public DemoApplication
{
    private:
		vk::RenderPass render_pass;

		vk::DescriptorSetLayout descriptor_set_layout;
		vk::PipelineLayout pipeline_layout;
		vk::Pipeline pipeline;

		vk::CommandPool command_pool;
		std::vector<vk::CommandBuffer> command_buffers;

		vk::DescriptorPool descriptor_pool;
		vk::DescriptorSet descriptor_set;

		const std::vector<Vertex> vertices = {
				{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
				{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
				{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
				{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
		};

		const std::vector<uint16_t> indices = {
				0, 1, 2, 2, 3, 0
		};

		vk::Buffer vertex_buffer;
		vk::DeviceMemory vertex_buffer_memory;

		vk::Buffer index_buffer;
		vk::DeviceMemory index_buffer_memory;

		vk::Buffer matrix_uniform_buffer;
		vk::DeviceMemory matrix_uniform_buffer_memory;

		vk::Image texture_image;
		vk::DeviceMemory texture_image_memory;
		vk::ImageView texture_image_view;
		vk::Sampler texture_sampler;

		void InitVulkan() override;

		void RecreateSwapchain() override;
		void CleanupSwapchain() override;

		void CleanupApplication() override;

		void CreateRenderPasses();

		void CreateDescriptorSetLayout();

		vk::ShaderModule CreateShaderModule(const std::vector<char> &code);
		void CreatePipeline();

		void CreateFramebuffers();

		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateMatrixUniformBuffer();

		void CreateCommandPool();
		void CreateCommandBuffers();

		void CreateDescriptorPool();
		void CreateDescriptorSet();

		void CreateTextureImage();
		void CreateTextureImageView();
		void CreateTextureSampler();

		void UpdateMatrixUniformBuffer();

		void DrawFrame(uint32_t image_index) override;
};

#endif
