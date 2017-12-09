
#ifndef LAVOS_FRAME_APPLICATION_H
#define LAVOS_FRAME_APPLICATION_H

#include <chrono>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <engine.h>
#include <renderer.h>

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

		bool swapchain_recreated;

		std::chrono::high_resolution_clock::time_point last_frame_time;
		float delta_time = 0.0f;

		virtual void InitWindow(int width, int height, std::string title);
		virtual void InitVulkan(bool enable_layers);

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

	public:
		WindowApplication(int width, int height, std::string title, bool enable_layers);
		~WindowApplication();

		void BeginFrame();
		void Update();
		void Render(lavos::Renderer *renderer);
		void EndFrame();

		bool GetSwapchainRecreated() const							{ return swapchain_recreated; }
		vk::Extent2D GetSwapchainExtent() const 					{ return swapchain_extent; };
		vk::Format GetSwapchainImageFormat() const					{ return swapchain_image_format; }
		std::vector<vk::ImageView> GetSwapchainImageViews() const	{ return swapchain_image_views; }

		GLFWwindow *GetWindow() const			{ return window; }

		lavos::Engine *GetEngine() const		{ return engine; }
};

}

#endif //LAVOS_FRAME_APPLICATION_H
