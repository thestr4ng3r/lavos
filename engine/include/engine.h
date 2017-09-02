
#ifndef VULKAN_ENGINE_H
#define VULKAN_ENGINE_H

#include <vulkan/vulkan.hpp>
#include <set>

namespace engine
{

class Engine
{
	public:
		struct CreateInfo
		{
			std::string app_info;
			bool enable_validation_layers = false;
			std::set<std::string> required_instance_extensions;
			std::set<std::string> required_device_extensions;

			CreateInfo() = default;
		};

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

	private:
		const CreateInfo info;

		QueueFamilyIndices queue_family_indices;

		vk::Instance instance;
		vk::DebugReportCallbackEXT debug_report_callback;

		vk::PhysicalDevice physical_device;
		vk::Device device;

		vk::Queue graphics_queue;
		vk::Queue present_queue;


		std::vector<const char *> GetRequiredInstanceExtensions();
		std::vector<const char *> GetRequiredDeviceExtensions();

		bool CheckValidationLayerSupport();

		void CreateInstance();
		void SetupDebugCallback();

		bool CheckDeviceExtensionSupport(vk::PhysicalDevice physical_device);
		bool IsPhysicalDeviceSuitable(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);
		void PickPhysicalDevice(vk::SurfaceKHR surface);
		QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);
		void CreateLogicalDevice(vk::SurfaceKHR surface);

	public:
		Engine(const CreateInfo &info);
		~Engine();

		void InitializeForSurface(vk::SurfaceKHR surface);

		const vk::Instance &GetVkInstance()	const					{ return instance; }
		const vk::PhysicalDevice &GetVkPhysicalDevice()	const		{ return physical_device; }
		const vk::Device &GetVkDevice() const						{ return device; }

		const QueueFamilyIndices &GetQueueFamilyIndices() const		{ return queue_family_indices; }
		const vk::Queue &GetGraphicsQueue()	const 					{ return graphics_queue; }
		const vk::Queue &GetPresentQueue() const					{ return present_queue; }
};

}

#endif //VULKAN_ENGINE_H
