
#ifndef _DEMO_COMMON_DEMO_APPLICATION_H
#define _DEMO_COMMON_DEMO_APPLICATION_H

#ifndef __ANDROID__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include <engine.h>

class DemoApplication
{
	protected:
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

		virtual void InitVulkan();
		virtual void MainLoop();
		virtual void DrawFrame();
		virtual void Cleanup();

		virtual void RecreateSwapchain();
		virtual void CleanupSwapchain();

	private:
		void InitWindow();

		void CreateEngine();

		void CreateSurface();
		vk::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &available_formats);
		vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR> &available_present_modes);
		vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);
		void CreateSwapchain();
		void CreateImageViews();


	public:
		virtual ~DemoApplication() = default;

		void Run();
};

#endif
