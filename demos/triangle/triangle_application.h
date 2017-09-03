
#ifndef VULKAN_VERTEXBUFFER_APPLICATION_H
#define VULKAN_VERTEXBUFFER_APPLICATION_H

#include <demo_application.h>
#include <vulkan/vulkan.hpp>
#include <engine.h>

#include "demo_application.h"


class TriangleApplication: public DemoApplication
{
    private:
		vk::RenderPass render_pass;
		vk::PipelineLayout pipeline_layout;
		vk::Pipeline pipeline;

		vk::CommandPool command_pool;
		std::vector<vk::CommandBuffer> command_buffers;


		void InitVulkan() override;

		void RecreateSwapchain() override;
		void CleanupSwapchain() override;

		void CleanupApplication() override;

		void CreateRenderPasses();

		vk::ShaderModule CreateShaderModule(const std::vector<char> &code);
		void CreatePipeline();


		void CreateFramebuffers();

		void CreateCommandPool();
		void CreateCommandBuffers();

		void DrawFrame(uint32_t image_index) override;
};

#endif
