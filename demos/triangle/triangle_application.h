
#ifndef VULKAN_TRIANGLE_APPLICATION_H
#define VULKAN_TRIANGLE_APPLICATION_H

#if not(__ANDROID__)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include <vulkan/vulkan.hpp>

struct QueueFamilyIndices
{
	int graphics_family = -1;
	int present_family = -1;

	bool IsComplete()
	{
		return graphics_family >= 0
				&& present_family >= 0;
	}
};

class TriangleApplication
{
    public:
        void Run();

    private:
#if not(__ANDROID__)
        GLFWwindow *window;
		static void OnWindowResized(GLFWwindow *window, int width, int height);
#endif

		vk::Instance instance;
		vk::DebugReportCallbackEXT debug_report_callback;

		vk::SurfaceKHR surface;

		vk::PhysicalDevice physical_device;
		vk::Device device;

		vk::Queue graphics_queue;
		vk::Queue present_queue;

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
		std::vector<const char *> GetRequiredExtensions();
		void SetupDebugCallback();
		bool CheckValidationLayerSupport();
		void CreateInstance();

		bool CheckDeviceExtensionSupport(vk::PhysicalDevice physical_device);
		bool IsPhysicalDeviceSuitable(vk::PhysicalDevice physical_device);
		void PickPhysicalDevice();
		QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice physical_device);
		void CreateLogicalDevice();

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
