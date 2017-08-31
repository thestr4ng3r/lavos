
#include <iostream>
#include <set>
#include <fstream>
#include "triangle_application.h"

static const int screen_width = 800;
static const int screen_height = 600;

static const std::vector<const char *> validation_layers = {
        "VK_LAYER_LUNARG_standard_validation"
};

static const std::vector<const char *> device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static const bool enable_validation_layers =
#ifdef NDEBUG
    false;
#else
	true;
#endif

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

void TriangleApplication::Run()
{
    InitWindow();
    InitVulkan();
    MainLoop();
    Cleanup();
}


void TriangleApplication::InitWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(screen_width, screen_height, "Triangle", nullptr, nullptr);
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
	CreatePipeline();
}


std::vector<const char *> TriangleApplication::GetRequiredExtensions()
{
    std::vector<const char *> extensions;

    unsigned int glfw_extensions_count;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

    for(unsigned int i=0; i<glfw_extensions_count; i++)
        extensions.push_back(glfw_extensions[i]);

    if(enable_validation_layers)
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    return extensions;
}


bool TriangleApplication::CheckValidationLayerSupport()
{
    auto layers_available = vk::enumerateInstanceLayerProperties();

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

    if(enable_validation_layers && !CheckValidationLayerSupport())
        throw std::runtime_error("validation layers requested, but not available!");

    create_info.setEnabledLayerCount(0);

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
		vk::DeviceQueueCreateInfo queue_create_info;
		queue_create_info.queueFamilyIndex = static_cast<uint32_t>(queue_family);
		queue_create_info.queueCount = 1;

		float queue_priority = 1.0f;
		queue_create_info.setPQueuePriorities(&queue_priority);

		queue_create_infos.push_back(queue_create_info);
	}

	vk::PhysicalDeviceFeatures features;

	vk::DeviceCreateInfo create_info;
	create_info.setQueueCreateInfoCount(static_cast<uint32_t>(queue_create_infos.size()));
	create_info.setPQueueCreateInfos(queue_create_infos.data());
	create_info.setPEnabledFeatures(&features);

	create_info.setEnabledExtensionCount(static_cast<uint32_t>(device_extensions.size()));
	create_info.setPpEnabledExtensionNames(device_extensions.data());

	if(enable_validation_layers)
	{
		create_info.setEnabledLayerCount(static_cast<uint32_t>(validation_layers.size()));
		create_info.setPpEnabledLayerNames(validation_layers.data());
	}
	else
	{
		create_info.setEnabledLayerCount(0);
	}

	device = physical_device.createDevice(create_info);

	graphics_queue = device.getQueue(static_cast<uint32_t>(queue_family_indices.graphics_family), 0);
	present_queue = device.getQueue(static_cast<uint32_t>(queue_family_indices.present_family), 0);
}

void TriangleApplication::CreateSurface()
{
	VkSurfaceKHR c_surface;
	if(glfwCreateWindowSurface(static_cast<VkInstance>(instance), window, nullptr, &c_surface) != VK_SUCCESS)
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

	vk::Extent2D extent(screen_width, screen_width);
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

	vk::SwapchainCreateInfoKHR create_info;
	create_info.surface = surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

	auto queue_family_indices = FindQueueFamilies(physical_device);
	uint32_t queue_family_indices_array[] = {static_cast<uint32_t>(queue_family_indices.graphics_family),
											 static_cast<uint32_t>(queue_family_indices.present_family)};

	if(queue_family_indices.graphics_family != queue_family_indices.present_family)
	{
		create_info.imageSharingMode = vk::SharingMode::eConcurrent;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices_array;
	}
	else
	{
		create_info.imageSharingMode = vk::SharingMode::eExclusive;
	}

	create_info.preTransform = surface_capabilities.currentTransform;
	create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = vk::SwapchainKHR(nullptr);

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
		vk::ImageViewCreateInfo create_info;
		create_info.setImage(swapchain_images[i]);
		create_info.setViewType(vk::ImageViewType::e2D);
		create_info.setFormat(swapchain_image_format);
		create_info.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
		swapchain_image_views[i] = device.createImageView(create_info);
	}
}

void TriangleApplication::CreateRenderPasses()
{
	vk::AttachmentDescription color_attachment;
	color_attachment.setFormat(swapchain_image_format);
	color_attachment.setSamples(vk::SampleCountFlagBits::e1);
	color_attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
	color_attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
	color_attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	color_attachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	color_attachment.setInitialLayout(vk::ImageLayout::eUndefined);
	color_attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentReference color_attachment_ref(0, vk::ImageLayout::eColorAttachmentOptimal);

	vk::SubpassDescription subpass;
	subpass.setColorAttachmentCount(1);
	subpass.setPColorAttachments(&color_attachment_ref);

	vk::RenderPassCreateInfo render_pass_info;
	render_pass_info.setAttachmentCount(1);
	render_pass_info.setPAttachments(&color_attachment);
	render_pass_info.setSubpassCount(1);
	render_pass_info.setPSubpasses(&subpass);

	render_pass = device.createRenderPass(render_pass_info);
}

vk::ShaderModule TriangleApplication::CreateShaderModule(const std::vector<char> &code)
{
	vk::ShaderModuleCreateInfo create_info;
	create_info.setCodeSize(code.size());
	create_info.setPCode(reinterpret_cast<const uint32_t *>(code.data()));
	return device.createShaderModule(create_info);
}

void TriangleApplication::CreatePipeline()
{
	auto vert_shader_module = CreateShaderModule(ReadFile("glsl/triangle.vert.spv"));
	auto frag_shader_module = CreateShaderModule(ReadFile("glsl/triangle.frag.spv"));

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

	vk::PipelineVertexInputStateCreateInfo vertex_input_info;

	vk::PipelineInputAssemblyStateCreateInfo input_assembly_info;
	input_assembly_info.setTopology(vk::PrimitiveTopology::eTriangleList);

	vk::Viewport viewport(0.0f, 0.0f, swapchain_extent.width, swapchain_extent.height, 0.0f, 1.0f);
	vk::Rect2D scissor({0, 0}, swapchain_extent);

	vk::PipelineViewportStateCreateInfo viewport_state_info;
	viewport_state_info.setViewportCount(1);
	viewport_state_info.setPViewports(&viewport);
	viewport_state_info.setScissorCount(1);
	viewport_state_info.setPScissors(&scissor);


	vk::PipelineRasterizationStateCreateInfo rasterizer_info;
	rasterizer_info.setDepthClampEnable(VK_FALSE);
	rasterizer_info.setRasterizerDiscardEnable(VK_FALSE);
	rasterizer_info.setPolygonMode(vk::PolygonMode::eFill);
	rasterizer_info.setLineWidth(1.0f);
	rasterizer_info.setCullMode(vk::CullModeFlagBits::eBack);
	rasterizer_info.setFrontFace(vk::FrontFace::eClockwise);
	rasterizer_info.setDepthBiasEnable(VK_FALSE);


	vk::PipelineMultisampleStateCreateInfo multisample_info;
	multisample_info.setSampleShadingEnable(VK_FALSE);
	multisample_info.setRasterizationSamples(vk::SampleCountFlagBits::e1);


	vk::PipelineColorBlendAttachmentState color_blend_attachment;
	color_blend_attachment.setColorWriteMask(vk::ColorComponentFlagBits::eR
											 | vk::ColorComponentFlagBits::eG
											 | vk::ColorComponentFlagBits::eB
											 | vk::ColorComponentFlagBits::eA);
	color_blend_attachment.setBlendEnable(VK_FALSE);

	vk::PipelineColorBlendStateCreateInfo color_blend_info;
	color_blend_info.setLogicOpEnable(VK_FALSE);
	color_blend_info.setAttachmentCount(1);
	color_blend_info.setPAttachments(&color_blend_attachment);


	vk::PipelineLayoutCreateInfo pipeline_layout_info;
	pipeline_layout = device.createPipelineLayout(pipeline_layout_info);


	vk::GraphicsPipelineCreateInfo pipeline_info;
	pipeline_info.setStageCount(2);
	pipeline_info.setPStages(shader_stages);
	pipeline_info.setPVertexInputState(&vertex_input_info);
	pipeline_info.setPInputAssemblyState(&input_assembly_info);
	pipeline_info.setPViewportState(&viewport_state_info);
	pipeline_info.setPRasterizationState(&rasterizer_info);
	pipeline_info.setPMultisampleState(&multisample_info);
	pipeline_info.setPDepthStencilState(nullptr);
	pipeline_info.setPColorBlendState(&color_blend_info);
	pipeline_info.setPDynamicState(nullptr);

	pipeline_info.setLayout(pipeline_layout);

	pipeline_info.setRenderPass(render_pass);
	pipeline_info.setSubpass(0);


	pipeline = device.createGraphicsPipeline(nullptr, pipeline_info);


	device.destroyShaderModule(vert_shader_module);
	device.destroyShaderModule(frag_shader_module);
}

void TriangleApplication::MainLoop()
{
    while(true)
    {
        if(glfwWindowShouldClose(window))
            break;


        glfwPollEvents();
    }
}

void TriangleApplication::Cleanup()
{
	device.destroyPipeline(pipeline);
	device.destroyPipelineLayout(pipeline_layout);
	device.destroyRenderPass(render_pass);

	for(const auto &image_view : swapchain_image_views)
		device.destroyImageView(image_view);

	device.destroySwapchainKHR(swapchain);

	device.destroy();

	if(enable_validation_layers)
		instance.destroyDebugReportCallbackEXT(debug_report_callback);

	instance.destroySurfaceKHR(surface);
    instance.destroy(nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}
