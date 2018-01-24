
#ifndef VULKAN_ENGINE_H
#define VULKAN_ENGINE_H

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

#include <set>

#include "lavos/buffer.h"
#include "lavos/texture.h"

namespace lavos
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

			bool enable_anisotropy = true;

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

		VmaAllocator allocator;

		vk::Queue graphics_queue;
		vk::Queue present_queue;


		vk::CommandPool transient_command_pool;


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

		void CreateAllocator();

		void CreateGlobalCommandPools();

	public:
		Engine(const CreateInfo &info);
		~Engine();

		void InitializeForSurface(vk::SurfaceKHR surface);
		void InitializeWithDevice(vk::PhysicalDevice physical_device, vk::Device device, vk::Queue graphics_queue, vk::Queue present_queue);

		const vk::Instance &GetVkInstance()	const					{ return instance; }
		const vk::PhysicalDevice &GetVkPhysicalDevice()	const		{ return physical_device; }
		const vk::Device &GetVkDevice() const						{ return device; }
		const VmaAllocator &GetVmaAllocator() const 				{ return allocator; };

		const QueueFamilyIndices &GetQueueFamilyIndices() const		{ return queue_family_indices; }
		const vk::Queue &GetGraphicsQueue()	const 					{ return graphics_queue; }
		const vk::Queue &GetPresentQueue() const					{ return present_queue; }

		uint32_t FindMemoryType(uint32_t type_filter, vk::MemoryPropertyFlags properties);

		vk::Format FindSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
		vk::Format FindDepthFormat();
		static bool HasStencilComponent(vk::Format format);

		vk::CommandBuffer BeginSingleTimeCommandBuffer();
		void EndSingleTimeCommandBuffer(vk::CommandBuffer command_buffer);

		Buffer CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage vma_usage, vk::SharingMode sharing_mode = vk::SharingMode::eExclusive);
		void DestroyBuffer(const Buffer &buffer);

		void *MapMemory(const VmaAllocation &allocation);
		void UnmapMemory(const VmaAllocation &allocation);

		void CopyBuffer(vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size);


		Image CreateImage(vk::ImageCreateInfo create_info, VmaMemoryUsage vma_usage);
		void DestroyImage(const Image &image);

		void DestroyTexture(const Texture &texture);

		Image Create2DImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, VmaMemoryUsage vma_usage, vk::SharingMode sharing_mode = vk::SharingMode::eExclusive);

		void TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout old_layout, vk::ImageLayout new_layout);

		void CopyBufferTo2DImage(vk::Buffer src_buffer, vk::Image dst_image, uint32_t width, uint32_t height);
};

}

#endif //VULKAN_ENGINE_H
