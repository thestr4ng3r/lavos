
#ifndef VULKAN_TRIANGLE_APPLICATION_H
#define VULKAN_TRIANGLE_APPLICATION_H

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

		vk::Semaphore image_available_semaphore;
		vk::Semaphore render_finished_semaphore;


		void InitVulkan() override;

		void RecreateSwapchain() override;
		void CleanupSwapchain() override;

		void Cleanup() override;

		void CreateRenderPasses();

		vk::ShaderModule CreateShaderModule(const std::vector<char> &code);
		void CreatePipeline();

		void CreateFramebuffers();

		void CreateCommandPool();
		void CreateCommandBuffers();

		void CreateSemaphores();

		void DrawFrame() override;
};

#endif
