
#include <iostream>
#include <set>
#include "triangle_application.h"

static const int screen_width = 800;
static const int screen_height = 600;

static const std::vector<const char *> validation_layers = {
        "VK_LAYER_LUNARG_standard_validation"
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

bool TriangleApplication::IsPhysicalDeviceSuitable(vk::PhysicalDevice physical_device)
{
	auto props = physical_device.getProperties();
	auto features = physical_device.getFeatures();

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

	create_info.setEnabledExtensionCount(0);

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
	device.destroy();

	if(enable_validation_layers)
		instance.destroyDebugReportCallbackEXT(debug_report_callback);

	vkDestroySurfaceKHR(static_cast<VkInstance>(instance), static_cast<VkSurfaceKHR>(surface), 0);

    instance.destroy(nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}
