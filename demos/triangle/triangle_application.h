
#ifndef VULKAN_TRIANGLE_APPLICATION_H
#define VULKAN_TRIANGLE_APPLICATION_H

#ifndef __ANDROID__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include <vulkan/vulkan.hpp>
#include <engine.h>


class TriangleApplication
{
    public:
        void Run();

    private:
#ifndef __ANDROID__
        GLFWwindow *window;
		static void OnWindowResized(GLFWwindow *window, int width, int height);
#endif

		engine::Engine *engine;

		vk::SurfaceKHR surface;

		vk::SwapchainKHR swapchain;
		vk::Format swapchain_image_format;
		vk::Extent2D swapchain_extent;
		std::vector<vk::Image> swapchain_images;
		std::vector<vk::ImageView> swapchain_image_views;
		std::vector<vk::Framebuffer> swapchain_framebuffers;

		vk::RenderPass render_pass;
		vk::PipelineLayout pipeline_layout;
		vk::Pipeline pipeline;

		vk::CommandPool command_pool;
		std::vector<vk::CommandBuffer> command_buffers;

		vk::Semaphore image_available_semaphore;
		vk::Semaphore render_finished_semaphore;

        void InitWindow();

        void InitVulkan();

		void CreateEngine();



		void CreateSurface();
		vk::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &available_formats);
		vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR> &available_present_modes);
		vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);
		void CreateSwapchain();
		void CreateImageViews();

		void RecreateSwapchain();

		void CreateRenderPasses();

		vk::ShaderModule CreateShaderModule(const std::vector<char> &code);
		void CreatePipeline();

		void CreateFramebuffers();

		void CreateCommandPool();
		void CreateCommandBuffers();

		void CreateSemaphores();

        void MainLoop();

		void CleanupSwapchain();
        void Cleanup();

		void DrawFrame();
};

#endif
