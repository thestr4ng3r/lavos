
#ifndef VULKAN_TRIANGLE_APPLICATION_H
#define VULKAN_TRIANGLE_APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
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
        GLFWwindow *window;

		vk::Instance instance;
		vk::DebugReportCallbackEXT debug_report_callback;

		vk::SurfaceKHR surface;

		vk::PhysicalDevice physical_device;
		vk::Device device;

		vk::Queue graphics_queue;
		vk::Queue present_queue;

        void InitWindow();

        void InitVulkan();
		std::vector<const char *> GetRequiredExtensions();
		void SetupDebugCallback();
		bool CheckValidationLayerSupport();
		void CreateInstance();

		bool IsPhysicalDeviceSuitable(vk::PhysicalDevice physical_device);
		void PickPhysicalDevice();
		QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice physical_device);
		void CreateLogicalDevice();

		void CreateSurface();

        void MainLoop();
        void Cleanup();
};

#endif
