
#include <iostream>
#include "engine.h"

using namespace lavos;

static const std::vector<const char *> validation_layers = {
#ifdef __ANDROID__
		"VK_LAYER_GOOGLE_threading",
		"VK_LAYER_LUNARG_core_validation",
		"VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_swapchain",
		"VK_LAYER_LUNARG_image",
		"VK_LAYER_LUNARG_parameter_validation",
		"VK_LAYER_GOOGLE_unique_objects"
#else
	"VK_LAYER_LUNARG_standard_validation",

	/*"VK_LAYER_RENDERDOC_Capture",*/
#endif
};




VkResult vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
										const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pCallback);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
		func(instance, callback, pAllocator);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
													uint64_t obj, size_t location, int32_t code,
													const char *layerPrefix, const char *msg, void *userData)
{
	std::cerr << "vulkan: " << msg << std::endl;
	return VK_FALSE;
}




Engine::Engine(const CreateInfo &info)
	: info(info)
{
	CreateInstance();
	SetupDebugCallback();
}

Engine::~Engine()
{
	vmaDestroyAllocator(allocator);

	if(debug_report_callback)
		instance.destroyDebugReportCallbackEXT(debug_report_callback);

	instance.destroy(nullptr);
}


std::vector<const char *> Engine::GetRequiredInstanceExtensions()
{
	std::vector<const char *> extensions;

	for(const auto &extension : info.required_instance_extensions)
		extensions.push_back(extension.c_str());

	if(info.enable_validation_layers)
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	return extensions;
}

std::vector<const char *> Engine::GetRequiredDeviceExtensions()
{
	std::vector<const char *> extensions;

	for(const auto &extension : info.required_device_extensions)
		extensions.push_back(extension.c_str());

	return extensions;
}

bool Engine::CheckValidationLayerSupport()
{
	std::cout << "Available Layers:" << std::endl;

	auto layers_available = vk::enumerateInstanceLayerProperties();

	for(const auto &layer_props : layers_available)
	{
		std::cout << "\t" << layer_props.layerName << std::endl;
	}

	for(const char *layer_name : validation_layers)
	{
		bool layer_found = false;

		for(const auto &layer_props : layers_available)
		{
			if(strcmp(layer_name, layer_props.layerName) == 0)
			{
				layer_found = true;
				break;
			}
		}

		if(!layer_found)
			return false;
	}

	return true;
}

void Engine::CreateInstance()
{
	vk::ApplicationInfo app_info(info.app_info.c_str(), VK_MAKE_VERSION(1, 0, 0),
								 "no engine", VK_MAKE_VERSION(1, 0, 0),
								 VK_API_VERSION_1_0);

	vk::InstanceCreateInfo create_info;
	create_info.setPApplicationInfo(&app_info);


	// extensions

	auto extensions_available = vk::enumerateInstanceExtensionProperties();
	std::cout << "Available Extensions:" << std::endl;
	for(const auto &extension : extensions_available)
		std::cout << "\t" << extension.extensionName << std::endl;

	auto required_extensions = GetRequiredInstanceExtensions();
	create_info.setEnabledExtensionCount(static_cast<uint32_t>(required_extensions.size()));
	create_info.setPpEnabledExtensionNames(required_extensions.data());


	// layers

	if(info.enable_validation_layers)
	{
		if(!CheckValidationLayerSupport())
			throw std::runtime_error("validation layers requested, but not available!");

		create_info.setEnabledLayerCount(static_cast<uint32_t>(validation_layers.size()));
		create_info.setPpEnabledLayerNames(validation_layers.data());
	}
	else
	{
		create_info.setEnabledLayerCount(0);
	}


	instance = vk::createInstance(create_info);
}



void Engine::SetupDebugCallback()
{
	if(!info.enable_validation_layers)
		return;

	vk::DebugReportCallbackCreateInfoEXT create_info(vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning,
													 DebugCallback, this);
	debug_report_callback = instance.createDebugReportCallbackEXT(create_info);
}



bool Engine::CheckDeviceExtensionSupport(vk::PhysicalDevice physical_device)
{
	auto available_extensions = physical_device.enumerateDeviceExtensionProperties();

	std::vector<const char *> device_extensions = GetRequiredDeviceExtensions();
	std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

	for(const auto &extension : available_extensions)
		required_extensions.erase(extension.extensionName);

	return required_extensions.empty();
}


void Engine::InitializeForSurface(vk::SurfaceKHR surface)
{
	PickPhysicalDevice(surface);
	CreateLogicalDevice(surface);
	CreateAllocator();
	CreateGlobalCommandPools();
}

void Engine::InitializeWithDevice(vk::PhysicalDevice physical_device, vk::Device device, vk::Queue graphics_queue, vk::Queue present_queue)
{
	this->physical_device = physical_device;
	this->device = device;
	this->graphics_queue = graphics_queue;
	this->present_queue = present_queue;

	queue_family_indices = FindQueueFamilies(physical_device, nullptr);

	CreateAllocator();
	CreateGlobalCommandPools();
}

bool Engine::IsPhysicalDeviceSuitable(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
{
	auto props = physical_device.getProperties();
	auto features = physical_device.getFeatures();


	if(info.enable_anisotropy && !features.samplerAnisotropy)
		return false;

	// extensions

	if(!CheckDeviceExtensionSupport(physical_device))
		return false;


	// surface

	auto surface_formats = physical_device.getSurfaceFormatsKHR(surface);
	auto surface_present_modes = physical_device.getSurfacePresentModesKHR(surface);

	if(surface_formats.empty() || surface_present_modes.empty())
		return false;

	return true;
}

void Engine::PickPhysicalDevice(vk::SurfaceKHR surface)
{
	auto physical_devices = instance.enumeratePhysicalDevices();
	if(physical_devices.empty())
		throw std::runtime_error("failed to find GPUs with Vulkan support!");


	std::cout << "Available Physical Devices:" << std::endl;
	for(const auto &physical_device : physical_devices)
	{
		auto props = physical_device.getProperties();
		std::cout << "\t" << props.deviceName << std::endl;
	}

	for(const auto &physical_device : physical_devices)
	{
		if(IsPhysicalDeviceSuitable(physical_device, surface))
		{
			this->physical_device = physical_device;
			break;
		}
	}

	if(!physical_device)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

Engine::QueueFamilyIndices Engine::FindQueueFamilies(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
{
	QueueFamilyIndices indices;

	auto queue_families = physical_device.getQueueFamilyProperties();
	int i = 0;
	for(const auto &queue_family : queue_families)
	{
		if(queue_family.queueCount > 0 && queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
			indices.graphics_family = i;

		if(queue_family.queueCount > 0 && (!surface || physical_device.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface)))
			indices.present_family = i;

		if(indices.IsComplete())
			break;

		i++;
	}

	return indices;
}

void Engine::CreateLogicalDevice(vk::SurfaceKHR surface)
{
	queue_family_indices = FindQueueFamilies(physical_device, surface);

	std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
	std::set<int> unique_queue_families = { queue_family_indices.graphics_family, queue_family_indices.present_family };

	for(int queue_family : unique_queue_families)
	{
		float queue_priority = 1.0f;

		auto queue_info = vk::DeviceQueueCreateInfo()
				.setQueueFamilyIndex(static_cast<uint32_t>(queue_family))
				.setQueueCount(1)
				.setPQueuePriorities(&queue_priority);

		queue_create_infos.push_back(queue_info);
	}

	auto features = vk::PhysicalDeviceFeatures()
		.setSamplerAnisotropy(info.enable_anisotropy ? VK_TRUE : VK_FALSE);


	std::vector<const char *> device_extensions = GetRequiredDeviceExtensions();

	auto create_info = vk::DeviceCreateInfo()
			.setQueueCreateInfoCount(static_cast<uint32_t>(queue_create_infos.size()))
			.setPQueueCreateInfos(queue_create_infos.data())
			.setPEnabledFeatures(&features)
			.setEnabledExtensionCount(static_cast<uint32_t>(device_extensions.size()))
			.setPpEnabledExtensionNames(device_extensions.data());

	if(info.enable_validation_layers)
	{
		create_info
				.setEnabledLayerCount(static_cast<uint32_t>(validation_layers.size()))
				.setPpEnabledLayerNames(validation_layers.data());
	}
	else
	{
		create_info.setEnabledLayerCount(0);
	}

	device = physical_device.createDevice(create_info);

	graphics_queue = device.getQueue(static_cast<uint32_t>(queue_family_indices.graphics_family), 0);
	present_queue = device.getQueue(static_cast<uint32_t>(queue_family_indices.present_family), 0);
}


void Engine::CreateAllocator()
{
	VmaAllocatorCreateInfo create_info;
	create_info.flags = 0;
	create_info.physicalDevice = physical_device;
	create_info.device = device;
	create_info.preferredLargeHeapBlockSize = 0;
	create_info.preferredSmallHeapBlockSize = 0;
	create_info.pAllocationCallbacks = 0;
	create_info.pDeviceMemoryCallbacks = 0;

	VkResult result = vmaCreateAllocator(&create_info, &allocator);

	if(result != VK_SUCCESS)
		throw std::runtime_error("failed to create allocator.");
}


void Engine::CreateGlobalCommandPools()
{
	auto command_pool_info = vk::CommandPoolCreateInfo()
			.setFlags(vk::CommandPoolCreateFlagBits::eTransient)
			.setQueueFamilyIndex(static_cast<uint32_t>(queue_family_indices.graphics_family));

	transient_command_pool = device.createCommandPool(command_pool_info);
}

vk::CommandBuffer Engine::BeginSingleTimeCommandBuffer()
{
	auto allocate_info = vk::CommandBufferAllocateInfo()
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandPool(transient_command_pool)
			.setCommandBufferCount(1);

	auto command_buffer = *device.allocateCommandBuffers(allocate_info).begin();

	command_buffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

	return command_buffer;
}

void Engine::EndSingleTimeCommandBuffer(vk::CommandBuffer command_buffer)
{
	command_buffer.end();

	auto submit_info = vk::SubmitInfo()
			.setCommandBufferCount(1)
			.setPCommandBuffers(&command_buffer);

	graphics_queue.submit(submit_info, nullptr);
	graphics_queue.waitIdle(); // TODO: use fence maybe?

	device.freeCommandBuffers(transient_command_pool, command_buffer);
}

uint32_t Engine::FindMemoryType(uint32_t type_filter, vk::MemoryPropertyFlags properties)
{
	auto memory_properties = physical_device.getMemoryProperties();

	for(uint32_t i=0; i<memory_properties.memoryTypeCount; i++)
	{
		if(type_filter & (1 << i)
		   && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

vk::Format Engine::FindSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
	for(auto format : candidates)
	{
		vk::FormatProperties props = physical_device.getFormatProperties(format);

		if(tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
			return format;

		if(tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
			return format;
	}

	throw std::runtime_error("failed to find supported format!");
}

vk::Format Engine::FindDepthFormat()
{
	return FindSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
							   vk::ImageTiling::eOptimal,
							   vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

bool Engine::HasStencilComponent(vk::Format format)
{
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

Buffer Engine::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage vma_usage, vk::SharingMode sharing_mode)
{
	auto create_info = vk::BufferCreateInfo()
			.setSize(size)
			.setUsage(usage)
			.setSharingMode(sharing_mode);

	VmaMemoryRequirements memory_requirements;
	memory_requirements.flags = 0;
	memory_requirements.usage = vma_usage;
	memory_requirements.requiredFlags = 0;
	memory_requirements.preferredFlags = 0;
	memory_requirements.pUserData = 0;

	VkBuffer buffer;
	VmaAllocation allocation;
	VkResult result = vmaCreateBuffer(allocator, reinterpret_cast<const VkBufferCreateInfo *>(&create_info), &memory_requirements, &buffer, &allocation, nullptr);

	if(result != VK_SUCCESS)
		throw std::runtime_error("Failed to create buffer.");

	return Buffer(buffer, allocation);
}

void Engine::DestroyBuffer(const Buffer &buffer)
{
	vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
}

Image Engine::CreateImage(vk::ImageCreateInfo create_info, VmaMemoryUsage vma_usage)
{
	VmaMemoryRequirements memory_requirements;
	memory_requirements.flags = 0;
	memory_requirements.usage = vma_usage;
	memory_requirements.requiredFlags = 0;
	memory_requirements.preferredFlags = 0;
	memory_requirements.pUserData = 0;

	VkImage image;
	VmaAllocation allocation;
	VkResult result = vmaCreateImage(allocator, reinterpret_cast<const VkImageCreateInfo *>(&create_info), &memory_requirements, &image, &allocation, nullptr);

	if(result != VK_SUCCESS)
		throw std::runtime_error("Failed to create image.");

	return Image(image, allocation, create_info.format);
}

void Engine::DestroyImage(const Image &image)
{
	vmaDestroyImage(allocator, image.image, image.allocation);
}

void Engine::DestroyTexture(const Texture &texture)
{
	DestroyImage(texture.image);
	device.destroyImageView(texture.image_view);
	device.destroySampler(texture.sampler);
}

void *Engine::MapMemory(const VmaAllocation &allocation)
{
	void *data;
	VkResult result = vmaMapMemory(allocator, allocation, &data);

	if(result != VK_SUCCESS)
		throw std::runtime_error("Failed to map memory.");

	return data;
}

void Engine::UnmapMemory(const VmaAllocation &allocation)
{
	vmaUnmapMemory(allocator, allocation);
}

void Engine::CopyBuffer(vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size)
{
	auto command_buffer = BeginSingleTimeCommandBuffer();
	command_buffer.copyBuffer(src_buffer, dst_buffer, vk::BufferCopy(0, 0, size));
	EndSingleTimeCommandBuffer(command_buffer);
}

Image Engine::Create2DImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
								vk::ImageUsageFlags usage, VmaMemoryUsage vma_usage, vk::SharingMode sharing_mode)
{
	auto image_info = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setExtent(vk::Extent3D(width, height, 1))
			.setMipLevels(1)
			.setArrayLayers(1)
			.setFormat(format)
			.setTiling(tiling)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setUsage(usage)
			.setSharingMode(sharing_mode)
			.setSamples(vk::SampleCountFlagBits::e1);

	return CreateImage(image_info, vma_usage);
}

void Engine::TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout old_layout,
								   vk::ImageLayout new_layout)
{
	auto command_buffer = BeginSingleTimeCommandBuffer();

	auto barrier = vk::ImageMemoryBarrier()
		.setOldLayout(old_layout)
		.setNewLayout(new_layout)
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setImage(image)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

	vk::PipelineStageFlags src_stage;
	vk::PipelineStageFlags dst_stage;

	if(old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.setSrcAccessMask(vk::AccessFlags())
			.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

		src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
		dst_stage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if(old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
			.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

		src_stage = vk::PipelineStageFlagBits::eTransfer;
		dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if(old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth);

		if(HasStencilComponent(format))
			barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;

		barrier.setSrcAccessMask(vk::AccessFlags())
			.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead);

		src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
		dst_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}

	command_buffer.pipelineBarrier(src_stage, dst_stage, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);

	EndSingleTimeCommandBuffer(command_buffer);
}

void Engine::CopyBufferTo2DImage(vk::Buffer src_buffer, vk::Image dst_image, uint32_t width, uint32_t height)
{
	auto command_buffer = BeginSingleTimeCommandBuffer();

	auto region = vk::BufferImageCopy()
		.setBufferOffset(0)
		.setBufferRowLength(0)
		.setBufferImageHeight(0)
		.setImageSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
		.setImageOffset(vk::Offset3D(0, 0, 0))
		.setImageExtent(vk::Extent3D(width, height, 1));

	command_buffer.copyBufferToImage(src_buffer, dst_image, vk::ImageLayout::eTransferDstOptimal, region);

	EndSingleTimeCommandBuffer(command_buffer);
}
