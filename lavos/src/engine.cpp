
#include "lavos/engine.h"
#include "lavos/log.h"
#include "lavos/vk_util.h"

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
	"VK_LAYER_KHRONOS_validation",

	/*"VK_LAYER_RENDERDOC_Capture",*/
#endif
};


VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, messenger, pAllocator);
}

VkResult vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo)
{
	auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
	if (func != nullptr)
		return func(device, pNameInfo);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}


static VkBool32 DebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
									 VkDebugUtilsMessageTypeFlagsEXT messageType,
									 const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
									 void *pUserData)
{
	LogLevel level;
	if((int)messageSeverity <= (int)vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
		level = LogLevel::Info;
	else if((int)messageSeverity <= (int)vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
		level = LogLevel::Warning;
	else
		level = LogLevel::Error;

	char type[4];
	type[0] = (messageType & (VkFlags)vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral) ? 'g' : '-';
	type[1] = (messageType & (VkFlags)vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation) ? 'v' : '-';
	type[2] = (messageType & (VkFlags)vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance) ? 'p' : '-';
	type[3] = '\0';

	LAVOS_LOGF(level, "VULKAN [%s]: %s", type, pCallbackData->pMessage);
	const char *padding = "              ";


	bool print_names = false;
	for(uint32_t i=0; i<pCallbackData->objectCount; i++)
	{
		if(pCallbackData->pObjects[i].pObjectName)
		{
			print_names = true;
			break;
		}
	}

	if(print_names)
	{
		LAVOS_LOGF(level, "%s  Named Objects:", padding);
		for(uint32_t i=0; i<pCallbackData->objectCount; i++)
		{
			auto &object = pCallbackData->pObjects[i];
			if(!object.pObjectName)
				continue;
			LAVOS_LOGF(level, "%s    %#lx = %s", padding, object.objectHandle, object.pObjectName);
		}
	}

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
	device.destroy(render_command_pool);
	device.destroy(transient_command_pool);

	vmaDestroyAllocator(allocator);

	device.destroy();

	if(debug_utils_messenger)
		instance.destroyDebugUtilsMessengerEXT(debug_utils_messenger);

	instance.destroy(nullptr);
}


std::vector<const char *> Engine::GetRequiredInstanceExtensions()
{
	std::vector<const char *> extensions;

	for(const auto &extension : info.required_instance_extensions)
		extensions.push_back(extension.c_str());

	if(info.enable_validation_layers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

std::vector<const char *> Engine::GetRequiredDeviceExtensions()
{
	std::vector<const char *> extensions;

	for(const auto &extension : info.required_device_extensions)
		extensions.push_back(extension.c_str());

	return extensions;
}

#ifdef LAVOS_ENABLE_RENDERDOC
#define LAVOS_VK_LAYER_RENDERDOC_Capture "VK_LAYER_RENDERDOC_Capture"
#endif

std::vector<const char *> Engine::EnableValidationLayers()
{
	auto layers_available = vk::enumerateInstanceLayerProperties();

	LAVOS_LOG(LogLevel::Debug, "Available Layers:");
	for(const auto &layer_props : layers_available)
		LAVOS_LOGF(LogLevel::Debug, "\t%s", layer_props.layerName.data());

	std::vector<const char *> layers_requested;
	std::vector<const char *> layers_enabled;

	if(info.enable_validation_layers)
		std::copy(validation_layers.begin(), validation_layers.end(), std::back_inserter(layers_requested));

#if LAVOS_ENABLE_RENDERDOC
	bool enable_renderdoc = info.enable_renderdoc;
	if(!info.enable_renderdoc)
	{
		char *renderdoc_env = std::getenv("LAVOS_ENABLE_RENDERDOC");
		enable_renderdoc = renderdoc_env && strcmp(renderdoc_env, "1") == 0;
		if(enable_renderdoc)
		LAVOS_LOG(LogLevel::Debug, "RenderDoc enabled from LAVOS_ENABLE_RENDERDOC environment variable.");
	}
	else
	{
		LAVOS_LOG(LogLevel::Debug, "RenderDoc enabled.");
	}

	if(enable_renderdoc)
	{
		bool already_inserted = false;
		for(auto layer_name : layers_requested)
		{
			if(strcmp(layer_name, LAVOS_VK_LAYER_RENDERDOC_Capture) == 0)
			{
				already_inserted = true;
				break;
			}
		}

		if(!already_inserted)
			layers_requested.push_back(LAVOS_VK_LAYER_RENDERDOC_Capture);
	}
#endif

	for(auto layer_name : layers_requested)
	{
		bool layer_found = false;

		for(const auto &layer_props : layers_available)
		{
			if(strcmp(layer_name, layer_props.layerName) == 0)
			{
				layer_found = true;
				layers_enabled.push_back(layer_name);
				break;
			}
		}

		if(!layer_found)
			LAVOS_LOGF(LogLevel::Error, "Layer %s requested, but not available.", layer_name);
	}

	return layers_enabled;
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
	LAVOS_LOG(LogLevel::Debug, "Available Extensions:");
	for(const auto &extension : extensions_available)
		LAVOS_LOGF(LogLevel::Debug, "\t%s", extension.extensionName.data());

	auto required_extensions = GetRequiredInstanceExtensions();
	create_info.setEnabledExtensionCount(static_cast<uint32_t>(required_extensions.size()));
	create_info.setPpEnabledExtensionNames(required_extensions.data());

	auto layers = EnableValidationLayers();
	create_info.setEnabledLayerCount(static_cast<uint32_t>(layers.size()));
	create_info.setPpEnabledLayerNames(layers.data());

	instance = vk::createInstance(create_info);
}



void Engine::SetupDebugCallback()
{
	if(!info.enable_validation_layers)
		return;

	debug_utils_messenger = instance.createDebugUtilsMessengerEXT(vk::DebugUtilsMessengerCreateInfoEXT()
		.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
			/*| vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo*/
			/*| vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose*/)
		.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
			| vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
			| vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
		.setPfnUserCallback(DebugUtilsMessengerCallback));
}



bool Engine::CheckDeviceExtensionSupport(vk::PhysicalDevice physical_device)
{
	auto available_extensions = physical_device.enumerateDeviceExtensionProperties();

	std::vector<const char *> device_extensions = GetRequiredDeviceExtensions();
	std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

	for(const auto &extension : available_extensions)
		required_extensions.erase(extension.extensionName.data());

	return required_extensions.empty();
}


void Engine::InitializeForSurface(vk::SurfaceKHR surface)
{
	PickPhysicalDevice(surface);
	CreateLogicalDevice();
	CreateAllocator();
	CreateGlobalCommandPools();
}

void Engine::InitializeWithPhysicalDevice(vk::PhysicalDevice physical_device)
{
	this->physical_device = physical_device;
	CreateLogicalDevice();
	CreateAllocator();
	CreateGlobalCommandPools();
}

void Engine::InitializeWithPhysicalDeviceIndex(unsigned int index)
{
	auto physical_devices = instance.enumeratePhysicalDevices();
	InitializeWithPhysicalDevice(physical_devices[index]);
}

void Engine::InitializeWithDevice(vk::PhysicalDevice physical_device, vk::Device device, vk::Queue graphics_queue)
{
	this->physical_device = physical_device;
	this->device = device;
	this->graphics_queue = graphics_queue;

	queue_family_indices = FindQueueFamilies(physical_device);

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


	LAVOS_LOG(LogLevel::Debug, "Available Physical Devices:");
	for(const auto &physical_device : physical_devices)
	{
		auto props = physical_device.getProperties();
		LAVOS_LOGF(LogLevel::Debug, "\t%s", props.deviceName.data());
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

Engine::QueueFamilyIndices Engine::FindQueueFamilies(vk::PhysicalDevice physical_device)
{
	QueueFamilyIndices indices;

	auto queue_families = physical_device.getQueueFamilyProperties();
	int i = 0;
	for(const auto &queue_family : queue_families)
	{
		if(queue_family.queueCount > 0 && queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
			indices.graphics_family = i;

		if(indices.IsComplete())
			break;

		i++;
	}

	return indices;
}

int Engine::FindPresentQueueFamily(vk::SurfaceKHR surface)
{
	auto queue_families = physical_device.getQueueFamilyProperties();
	int i = 0;
	for(const auto &queue_family : queue_families)
	{
		if(queue_family.queueCount > 0 && physical_device.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface))
			return i;

		i++;
	}

	return -1;
}


void Engine::CreateLogicalDevice()
{
	queue_family_indices = FindQueueFamilies(physical_device);

	std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
	std::set<int> unique_queue_families = { queue_family_indices.graphics_family };

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
}


void Engine::CreateAllocator()
{
	VmaAllocatorCreateInfo create_info = {};
	create_info.physicalDevice = physical_device;
	create_info.device = device;

	VkResult result = vmaCreateAllocator(&create_info, &allocator);

	if(result != VK_SUCCESS)
		throw std::runtime_error("failed to create allocator.");
}


void Engine::CreateGlobalCommandPools()
{
	transient_command_pool = device.createCommandPool(
			vk::CommandPoolCreateInfo()
					.setFlags(vk::CommandPoolCreateFlagBits::eTransient)
					.setQueueFamilyIndex(static_cast<uint32_t>(queue_family_indices.graphics_family)));

	render_command_pool = device.createCommandPool(
			vk::CommandPoolCreateInfo()
					.setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
					.setQueueFamilyIndex(static_cast<uint32_t>(queue_family_indices.graphics_family)));
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

Buffer *Engine::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage vma_usage, vk::SharingMode sharing_mode)
{
	auto create_info = vk::BufferCreateInfo()
			.setSize(size)
			.setUsage(usage)
			.setSharingMode(sharing_mode);

	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = vma_usage;

	VkBuffer buffer;
	VmaAllocation allocation;
	VkResult result = vmaCreateBuffer(allocator, reinterpret_cast<const VkBufferCreateInfo *>(&create_info), &alloc_info, &buffer, &allocation, nullptr);

	if(result != VK_SUCCESS)
		throw std::runtime_error("Failed to create buffer.");

	return new Buffer(this, buffer, allocation);
}

void Engine::DestroyBuffer(vk::Buffer buffer, VmaAllocation allocation)
{
	vmaDestroyBuffer(allocator, buffer, allocation);
}

Image Engine::CreateImage(vk::ImageCreateInfo create_info, VmaMemoryUsage vma_usage)
{
	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = vma_usage;

	VkImage image;
	VmaAllocation allocation;
	VkResult result = vmaCreateImage(allocator, reinterpret_cast<const VkImageCreateInfo *>(&create_info), &alloc_info, &image, &allocation, nullptr);

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
								   vk::ImageLayout new_layout, vk::ImageAspectFlags aspect_mask)
{
	auto command_buffer = BeginSingleTimeCommandBuffer();

	auto barrier = vk::ImageMemoryBarrier()
		.setOldLayout(old_layout)
		.setNewLayout(new_layout)
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setImage(image)
		.setSubresourceRange(vk::ImageSubresourceRange(aspect_mask, 0, 1, 0, 1));

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

void Engine::CopyBufferTo2DImage(vk::Buffer src_buffer, vk::Image dst_image, uint32_t width, uint32_t height, vk::ImageAspectFlags aspect_mask)
{
	auto command_buffer = BeginSingleTimeCommandBuffer();

	auto region = vk::BufferImageCopy()
		.setBufferOffset(0)
		.setBufferRowLength(0)
		.setBufferImageHeight(0)
		.setImageSubresource(vk::ImageSubresourceLayers(aspect_mask, 0, 0, 1))
		.setImageOffset(vk::Offset3D(0, 0, 0))
		.setImageExtent(vk::Extent3D(width, height, 1));

	command_buffer.copyBufferToImage(src_buffer, dst_image, vk::ImageLayout::eTransferDstOptimal, region);

	EndSingleTimeCommandBuffer(command_buffer);
}
