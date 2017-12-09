
#ifndef LAVOS_FRAME_APPLICATION_H
#define LAVOS_FRAME_APPLICATION_H

#ifndef __ANDROID__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include <engine.h>


namespace lavosframe
{

class WindowApplication
{
	private:
		GLFWwindow *window;
		static void OnWindowResized(GLFWwindow *window, int width, int height);

		lavos::Engine *engine;

		vk::SurfaceKHR surface;

		vk::SwapchainKHR swapchain;
		vk::Format swapchain_image_format;
		vk::Extent2D swapchain_extent;
		std::vector<vk::Image> swapchain_images;
		std::vector<vk::ImageView> swapchain_image_views;

		vk::Semaphore image_available_semaphore;
		vk::Semaphore render_finished_semaphore;


		virtual void InitWindow(int width, int height, std::string title);
		virtual void InitVulkan(bool enable_layers);

		virtual void Update(float delta_time) {};
		virtual void DrawFrame(uint32_t image_index) {};
		virtual void CleanupApplication() {};

		virtual void RecreateSwapchain();
		virtual void CleanupSwapchain();


		void CreateEngine(bool enable_layers);

		void CreateSurface();
		vk::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &available_formats);
		vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR> &available_present_modes);
		vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);
		void CreateSwapchain();
		void Cleanup();
		void CreateImageViews();

		void CreateSemaphores();

		void DrawAndPresentFrame();

	public:
		WindowApplication(int width, int height, std::string title, bool enable_layers);
		~WindowApplication();

		void RunMainLoop();
};

}

#endif //LAVOS_FRAME_APPLICATION_H
