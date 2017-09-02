
#if defined(__ANDROID__)
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <common.h>
#endif

#include <iostream>
#include <set>
#include <fstream>
#include "triangle_application.h"


static const int screen_width = 800;
static const int screen_height = 600;

static const std::vector<const char *> validation_layers = {
        //"VK_LAYER_LUNARG_standard_validation"

		"VK_LAYER_GOOGLE_threading",
		"VK_LAYER_LUNARG_core_validation",
		"VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_swapchain",
		"VK_LAYER_LUNARG_image",
		"VK_LAYER_LUNARG_parameter_validation",
		"VK_LAYER_GOOGLE_unique_objects"
};

static const std::vector<const char *> device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static const bool enable_validation_layers = true;
/*#ifdef NDEBUG
    false;
#else
	true;
#endif*/

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
    std::cerr << "validation layer: " << msg << std::endl;
    return VK_FALSE;
}

std::vector<char> ReadFile(const std::string &filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if(!file.is_open())
		throw std::runtime_error("failed to open file!");

	size_t size = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(size);

	file.seekg(0);
	file.read(buffer.data(), size);
	file.close();

	return buffer;
}

std::vector<char> ReadSPIRVShader(const std::string shader)
{
#if defined(__ANDROID__)
	return AndroidReadSPIRVShader(shader);
#else
	return ReadFile("glsl/" + shader + ".spv");
#endif
}

void TriangleApplication::Run()
{
    InitWindow();
    InitVulkan();
    MainLoop();
    Cleanup();
}


void TriangleApplication::InitWindow()
{
#if not(__ANDROID__)
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(screen_width, screen_height, "Triangle", nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, TriangleApplication::OnWindowResized);
#endif
}


void TriangleApplication::InitVulkan()
{
    CreateInstance();
    SetupDebugCallback();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapchain();
	CreateImageViews();
	CreateRenderPasses();
	CreatePipeline();
	CreateFramebuffers();
	CreateCommandPool();
	CreateCommandBuffers();
	CreateSemaphores();
}

std::vector<const char *> TriangleApplication::GetRequiredExtensions()
{
    std::vector<const char *> extensions;

#if defined(__ANDROID__)
	extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	extensions.push_back("VK_KHR_android_surface");
#else
    unsigned int glfw_extensions_count;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

    for(unsigned int i=0; i<glfw_extensions_count; i++)
        extensions.push_back(glfw_extensions[i]);
#endif

	if(enable_validation_layers)
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    return extensions;
}


bool TriangleApplication::CheckValidationLayerSupport()
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

void TriangleApplication::CreateInstance()
{
    vk::ApplicationInfo app_info("Triangle", VK_MAKE_VERSION(1, 0, 0),
                                "no engine", VK_MAKE_VERSION(1, 0, 0),
                                VK_API_VERSION_1_0);

    vk::InstanceCreateInfo create_info;
    create_info.setPApplicationInfo(&app_info);


    // extensions

    auto extensions_available = vk::enumerateInstanceExtensionProperties();
    std::cout << "Available Extensions:" << std::endl;
    for(const auto &extension : extensions_available)
        std::cout << "\t" << extension.extensionName << std::endl;

    auto required_extensions = GetRequiredExtensions();
    create_info.setEnabledExtensionCount(static_cast<uint32_t>(required_extensions.size()));
    create_info.setPpEnabledExtensionNames(required_extensions.data());


    // layers

	if(enable_validation_layers)
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

void TriangleApplication::SetupDebugCallback()
{
    if(!enable_validation_layers)
        return;

    vk::DebugReportCallbackCreateInfoEXT create_info(vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning,
                                                     DebugCallback);
	debug_report_callback = instance.createDebugReportCallbackEXT(create_info);
}

bool TriangleApplication::CheckDeviceExtensionSupport(vk::PhysicalDevice physical_device)
{
	auto available_extensions = physical_device.enumerateDeviceExtensionProperties();

	std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

	for(const auto &extension : available_extensions)
		required_extensions.erase(extension.extensionName);

	return required_extensions.empty();
}

bool TriangleApplication::IsPhysicalDeviceSuitable(vk::PhysicalDevice physical_device)
{
	auto props = physical_device.getProperties();
	auto features = physical_device.getFeatures();


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

void TriangleApplication::PickPhysicalDevice()
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
		if(IsPhysicalDeviceSuitable(physical_device))
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

QueueFamilyIndices TriangleApplication::FindQueueFamilies(vk::PhysicalDevice physical_device)
{
	QueueFamilyIndices indices;

	auto queue_families = physical_device.getQueueFamilyProperties();
	int i = 0;
	for(const auto &queue_family : queue_families)
	{
		if(queue_family.queueCount > 0 && queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
			indices.graphics_family = i;

		if(queue_family.queueCount > 0 && physical_device.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface))
			indices.present_family = i;

		if(indices.IsComplete())
			break;

		i++;
	}

	return indices;
}

void TriangleApplication::CreateLogicalDevice()
{
	QueueFamilyIndices queue_family_indices = FindQueueFamilies(physical_device);

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

	vk::PhysicalDeviceFeatures features;

	auto create_info = vk::DeviceCreateInfo()
		.setQueueCreateInfoCount(static_cast<uint32_t>(queue_create_infos.size()))
		.setPQueueCreateInfos(queue_create_infos.data())
		.setPEnabledFeatures(&features)
		.setEnabledExtensionCount(static_cast<uint32_t>(device_extensions.size()))
		.setPpEnabledExtensionNames(device_extensions.data());

	if(enable_validation_layers)
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

#if defined(__ANDROID__)
#include <android/native_window.h>

typedef VkFlags VkAndroidSurfaceCreateFlagsKHR;

typedef struct VkAndroidSurfaceCreateInfoKHR {
	VkStructureType                   sType;
	const void*                       pNext;
	VkAndroidSurfaceCreateFlagsKHR    flags;
	struct ANativeWindow*             window;
} VkAndroidSurfaceCreateInfoKHR;

typedef VkResult (VKAPI_PTR *PFN_vkCreateAndroidSurfaceKHR)(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif

void TriangleApplication::CreateSurface()
{
	VkSurfaceKHR c_surface;

#if defined(__ANDROID__)
	auto CreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR) vkGetInstanceProcAddr(static_cast<VkInstance>(instance), "vkCreateAndroidSurfaceKHR");

    VkAndroidSurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.window = AndroidGetApplicationWindow();
    VkResult result = CreateAndroidSurfaceKHR(static_cast<VkInstance>(instance), &createInfo, nullptr, &c_surface);
#else
	vkResult result = glfwCreateWindowSurface(static_cast<VkInstance>(instance), window, nullptr, &c_surface);
#endif

	if(result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

	surface = vk::SurfaceKHR(c_surface);

}

vk::SurfaceFormatKHR TriangleApplication::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &available_formats)
{
	const auto preferred_format = vk::Format::eB8G8R8A8Unorm;
	const auto preferred_color_space = vk::ColorSpaceKHR::eSrgbNonlinear;

	if(available_formats.size() == 1 && available_formats[0].format == vk::Format::eUndefined)
		return { preferred_format, preferred_color_space };

	for(const auto &format : available_formats)
	{
		if(format.format == preferred_format && format.colorSpace == preferred_color_space)
			return format;
	}

	return available_formats[0];
}

vk::PresentModeKHR TriangleApplication::ChoosePresentMode(const std::vector<vk::PresentModeKHR> &available_present_modes)
{
	vk::PresentModeKHR best_mode = vk::PresentModeKHR::eFifo;

	for(const auto &present_mode : available_present_modes)
	{
		if(present_mode == vk::PresentModeKHR::eMailbox)
			return present_mode;

		if(present_mode == vk::PresentModeKHR::eImmediate)
			best_mode = present_mode;
	}

	return best_mode;
}

vk::Extent2D TriangleApplication::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities)
{
	if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;

	int width, height;
#if defined(__ANDROID__)
	AndroidGetWindowSize(&width, &height);
#else
	glfwGetWindowSize(window, &width, &height);
#endif

	vk::Extent2D extent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
	extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));
	return extent;
}

void TriangleApplication::CreateSwapchain()
{
	auto surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
	auto surface_formats = physical_device.getSurfaceFormatsKHR(surface);
	auto surface_present_modes = physical_device.getSurfacePresentModesKHR(surface);

	auto surface_format = ChooseSurfaceFormat(surface_formats);
	auto present_mode = ChoosePresentMode(surface_present_modes);
	auto extent = ChooseSwapExtent(surface_capabilities);

	uint32_t image_count = surface_capabilities.minImageCount + 1;
	if(surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount)
		image_count = surface_capabilities.maxImageCount;

	auto create_info = vk::SwapchainCreateInfoKHR()
		.setSurface(surface)
		.setMinImageCount(image_count)
		.setImageFormat(surface_format.format)
		.setImageColorSpace(surface_format.colorSpace)
		.setImageExtent(extent)
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	auto queue_family_indices = FindQueueFamilies(physical_device);
	uint32_t queue_family_indices_array[] = {static_cast<uint32_t>(queue_family_indices.graphics_family),
											 static_cast<uint32_t>(queue_family_indices.present_family)};

	if(queue_family_indices.graphics_family != queue_family_indices.present_family)
	{
		create_info.setImageSharingMode(vk::SharingMode::eConcurrent)
			.setQueueFamilyIndexCount(2)
			.setPQueueFamilyIndices(queue_family_indices_array);
	}
	else
	{
		create_info.setImageSharingMode(vk::SharingMode::eExclusive);
	}

	create_info.setPreTransform(surface_capabilities.currentTransform)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(present_mode)
		.setClipped(VK_TRUE);
		//.setOldSwapchain(vk::SwapchainKHR(nullptr));

	swapchain = device.createSwapchainKHR(create_info);
	swapchain_images = device.getSwapchainImagesKHR(swapchain);

	swapchain_image_format = surface_format.format;
	swapchain_extent = extent;
}

void TriangleApplication::CreateImageViews()
{
	swapchain_image_views.resize(swapchain_images.size());

	for(size_t i=0; i<swapchain_images.size(); i++)
	{
		swapchain_image_views[i] = device.createImageView(
				vk::ImageViewCreateInfo()
						.setImage(swapchain_images[i])
						.setViewType(vk::ImageViewType::e2D)
						.setFormat(swapchain_image_format)
						.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
	}
}

void TriangleApplication::RecreateSwapchain()
{
	device.waitIdle();

	CleanupSwapchain();

	CreateSwapchain();
	CreateImageViews();
	CreateRenderPasses();
	CreatePipeline();
	CreateFramebuffers();
	CreateCommandBuffers();
}

void TriangleApplication::CreateRenderPasses()
{
	 auto color_attachment = vk::AttachmentDescription()
		.setFormat(swapchain_image_format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentReference color_attachment_ref(0, vk::ImageLayout::eColorAttachmentOptimal);

	auto subpass = vk::SubpassDescription()
		.setColorAttachmentCount(1)
		.setPColorAttachments(&color_attachment_ref);

	auto subpass_dependency = vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			//.setSrcAccessMask(0)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	render_pass = device.createRenderPass(
			vk::RenderPassCreateInfo()
				.setAttachmentCount(1)
				.setPAttachments(&color_attachment)
				.setSubpassCount(1)
				.setPSubpasses(&subpass)
				.setDependencyCount(1)
				.setPDependencies(&subpass_dependency));
}

vk::ShaderModule TriangleApplication::CreateShaderModule(const std::vector<char> &code)
{
	return device.createShaderModule(
			vk::ShaderModuleCreateInfo()
					.setCodeSize(code.size())
					.setPCode(reinterpret_cast<const uint32_t *>(code.data())));
}

void TriangleApplication::CreatePipeline()
{
	auto vert_shader_module = CreateShaderModule(ReadSPIRVShader("triangle.vert"));
	auto frag_shader_module = CreateShaderModule(ReadSPIRVShader("triangle.frag"));

	vk::PipelineShaderStageCreateInfo shader_stages[] = {
			vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(),
											  vk::ShaderStageFlagBits::eVertex,
											  vert_shader_module,
											  "main"),
			vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(),
											  vk::ShaderStageFlagBits::eFragment,
											  frag_shader_module,
											  "main")
	};

	auto vertex_input_info = vk::PipelineVertexInputStateCreateInfo();

	auto input_assembly_info = vk::PipelineInputAssemblyStateCreateInfo()
		.setTopology(vk::PrimitiveTopology::eTriangleList);

	vk::Viewport viewport(0.0f, 0.0f, swapchain_extent.width, swapchain_extent.height, 0.0f, 1.0f);
	vk::Rect2D scissor({0, 0}, swapchain_extent);

	auto viewport_state_info = vk::PipelineViewportStateCreateInfo()
		.setViewportCount(1)
		.setPViewports(&viewport)
		.setScissorCount(1)
		.setPScissors(&scissor);

	auto rasterizer_info = vk::PipelineRasterizationStateCreateInfo()
		.setDepthClampEnable(VK_FALSE)
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1.0f)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eClockwise)
		.setDepthBiasEnable(VK_FALSE);


	auto multisample_info = vk::PipelineMultisampleStateCreateInfo()
		.setSampleShadingEnable(VK_FALSE)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);


	auto color_blend_attachment = vk::PipelineColorBlendAttachmentState()
		.setColorWriteMask(vk::ColorComponentFlagBits::eR
						   | vk::ColorComponentFlagBits::eG
						   | vk::ColorComponentFlagBits::eB
						   | vk::ColorComponentFlagBits::eA)
		.setBlendEnable(VK_FALSE);

	auto color_blend_info = vk::PipelineColorBlendStateCreateInfo()
		.setLogicOpEnable(VK_FALSE)
		.setAttachmentCount(1)
		.setPAttachments(&color_blend_attachment);


	auto pipeline_layout_info = vk::PipelineLayoutCreateInfo();
	pipeline_layout = device.createPipelineLayout(pipeline_layout_info);


	auto pipeline_info = vk::GraphicsPipelineCreateInfo()
		.setStageCount(2)
		.setPStages(shader_stages)
		.setPVertexInputState(&vertex_input_info)
		.setPInputAssemblyState(&input_assembly_info)
		.setPViewportState(&viewport_state_info)
		.setPRasterizationState(&rasterizer_info)
		.setPMultisampleState(&multisample_info)
		.setPDepthStencilState(nullptr)
		.setPColorBlendState(&color_blend_info)
		.setPDynamicState(nullptr)
		.setLayout(pipeline_layout)
		.setRenderPass(render_pass)
		.setSubpass(0);


	pipeline = device.createGraphicsPipeline(vk::PipelineCache() /*nullptr*/, pipeline_info);


	device.destroyShaderModule(vert_shader_module);
	device.destroyShaderModule(frag_shader_module);
}

void TriangleApplication::CreateFramebuffers()
{
	swapchain_framebuffers.resize(swapchain_image_views.size());

	for(size_t i=0; i<swapchain_image_views.size(); i++)
	{
		vk::ImageView attachments[] = {
			swapchain_image_views[i]
		};

		auto framebuffer_info = vk::FramebufferCreateInfo()
			.setRenderPass(render_pass)
			.setAttachmentCount(1)
			.setPAttachments(attachments)
			.setWidth(swapchain_extent.width)
			.setHeight(swapchain_extent.height)
			.setLayers(1);

		swapchain_framebuffers[i] = device.createFramebuffer(framebuffer_info);
	}
}

void TriangleApplication::CreateCommandPool()
{
	auto queue_family_indices = FindQueueFamilies(physical_device);

	auto command_pool_info = vk::CommandPoolCreateInfo()
		.setQueueFamilyIndex(static_cast<uint32_t>(queue_family_indices.graphics_family));

	command_pool = device.createCommandPool(command_pool_info);
}

void TriangleApplication::CreateCommandBuffers()
{
	command_buffers = device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
					.setCommandPool(command_pool)
					.setLevel(vk::CommandBufferLevel::ePrimary)
					.setCommandBufferCount(static_cast<uint32_t>(swapchain_image_views.size())));

	for(size_t i=0; i<command_buffers.size(); i++)
	{
		const auto &command_buffer = command_buffers[i];

		command_buffer.begin({ vk::CommandBufferUsageFlagBits::eSimultaneousUse });

		vk::ClearValue clear_color_value = vk::ClearColorValue(std::array<float, 4>{{ 0.0f, 0.0f, 0.0f, 1.0f }});

		command_buffer.beginRenderPass(
				vk::RenderPassBeginInfo()
					.setRenderPass(render_pass)
					.setFramebuffer(swapchain_framebuffers[i])
					.setRenderArea(vk::Rect2D({ 0, 0 }, swapchain_extent))
					.setClearValueCount(1)
					.setPClearValues(&clear_color_value),
				vk::SubpassContents::eInline);

		command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
		command_buffer.draw(3, 1, 0, 0);

		command_buffer.endRenderPass();

		command_buffer.end();
	}
}

void TriangleApplication::CreateSemaphores()
{
	image_available_semaphore = device.createSemaphore(vk::SemaphoreCreateInfo());
	render_finished_semaphore = device.createSemaphore(vk::SemaphoreCreateInfo());
}

void TriangleApplication::MainLoop()
{
    while(true)
    {
#if defined(__ANDROID__)
		// TODO: events?
#else
        if(glfwWindowShouldClose(window))
            break;

        glfwPollEvents();
#endif

		DrawFrame();
    }

	device.waitIdle();
}

void TriangleApplication::CleanupSwapchain()
{
	for(vk::Framebuffer framebuffer : swapchain_framebuffers)
		device.destroyFramebuffer(framebuffer);

	device.freeCommandBuffers(command_pool, command_buffers);

	device.destroyPipeline(pipeline);
	device.destroyPipelineLayout(pipeline_layout);
	device.destroyRenderPass(render_pass);

	for(const auto &image_view : swapchain_image_views)
		device.destroyImageView(image_view);

	device.destroySwapchainKHR(swapchain);
}

void TriangleApplication::Cleanup()
{
	CleanupSwapchain();

	device.destroySemaphore(image_available_semaphore);
	device.destroySemaphore(render_finished_semaphore);

	device.destroyCommandPool(command_pool);

	device.destroy();

	if(enable_validation_layers)
		instance.destroyDebugReportCallbackEXT(debug_report_callback);

	instance.destroySurfaceKHR(surface);
    instance.destroy(nullptr);

#if not(__ANDROID__)
    glfwDestroyWindow(window);
    glfwTerminate();
#endif
}

void TriangleApplication::DrawFrame()
{
	auto image_index_result = device.acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), image_available_semaphore, vk::Fence() /*nullptr*/);

	if(image_index_result.result == vk::Result::eErrorOutOfDateKHR)
	{
		RecreateSwapchain();
		return;
	}
	else if(image_index_result.result != vk::Result::eSuccess && image_index_result.result != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	uint32_t image_index = image_index_result.value;

	vk::Semaphore wait_semaphores[] = { image_available_semaphore };
	vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	vk::Semaphore signal_semaphores[] = { render_finished_semaphore };

	graphics_queue.submit(
			vk::SubmitInfo()
				  .setWaitSemaphoreCount(1)
				  .setPWaitSemaphores(wait_semaphores)
				  .setPWaitDstStageMask(wait_stages)
				  .setCommandBufferCount(1)
				  .setPCommandBuffers(&command_buffers[image_index])
				  .setSignalSemaphoreCount(1)
				  .setPSignalSemaphores(signal_semaphores),
			vk::Fence() /*nullptr*/);

	auto present_result = present_queue.presentKHR(vk::PresentInfoKHR()
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(signal_semaphores)
		.setSwapchainCount(1)
		.setPSwapchains(&swapchain)
		.setPImageIndices(&image_index));

	if(present_result == vk::Result::eErrorOutOfDateKHR || present_result == vk::Result::eSuboptimalKHR)
	{
		RecreateSwapchain();
	}
	else if(present_result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	present_queue.waitIdle();
}

#if not(__ANDROID__)
void TriangleApplication::OnWindowResized(GLFWwindow *window, int width, int height)
{
	if(width == 0 || height == 0)
		return;

	auto *app = reinterpret_cast<TriangleApplication *>(glfwGetWindowUserPointer(window));
	app->RecreateSwapchain();
}
#endif