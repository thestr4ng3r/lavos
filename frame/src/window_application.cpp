
#include <chrono>
#include "window_application.h"

using namespace lavosframe;

WindowApplication::WindowApplication(int width, int height, std::string title, bool enable_layers)
{
	InitWindow(width, height, title);
	InitVulkan(enable_layers);
}

WindowApplication::~WindowApplication()
{
	Cleanup();
}

void WindowApplication::InitWindow(int width, int height, std::string title)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, WindowApplication::OnWindowResized);
}



void WindowApplication::InitVulkan(bool enable_layers)
{
	CreateEngine(enable_layers);

	CreateSwapchain();
	CreateImageViews();
	CreateSemaphores();
}


void WindowApplication::CreateEngine(bool enable_layers)
{
	lavos::Engine::CreateInfo create_info;

	unsigned int glfw_extensions_count;
	const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

	for(unsigned int i=0; i<glfw_extensions_count; i++)
		create_info.required_instance_extensions.insert(std::string(glfw_extensions[i]));

	create_info.enable_validation_layers = enable_layers;

	create_info.required_device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	engine = new lavos::Engine(create_info);

	CreateSurface();

	engine->InitializeForSurface(surface);
}

void WindowApplication::CreateSurface()
{
	VkSurfaceKHR c_surface;

	VkResult result = glfwCreateWindowSurface(static_cast<VkInstance>(engine->GetVkInstance()), window, nullptr, &c_surface);

	if(result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

	surface = vk::SurfaceKHR(c_surface);

}

vk::SurfaceFormatKHR WindowApplication::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &available_formats)
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

vk::PresentModeKHR WindowApplication::ChoosePresentMode(const std::vector<vk::PresentModeKHR> &available_present_modes)
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

vk::Extent2D WindowApplication::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities)
{
	if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;

	int width, height;
	glfwGetWindowSize(window, &width, &height);

	vk::Extent2D extent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
	extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));
	return extent;
}

void WindowApplication::CreateSwapchain()
{
	auto surface_capabilities = engine->GetVkPhysicalDevice().getSurfaceCapabilitiesKHR(surface);
	auto surface_formats = engine->GetVkPhysicalDevice().getSurfaceFormatsKHR(surface);
	auto surface_present_modes = engine->GetVkPhysicalDevice().getSurfacePresentModesKHR(surface);

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

	auto queue_family_indices = engine->GetQueueFamilyIndices();
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

	swapchain = engine->GetVkDevice().createSwapchainKHR(create_info);
	swapchain_images = engine->GetVkDevice().getSwapchainImagesKHR(swapchain);

	swapchain_image_format = surface_format.format;
	swapchain_extent = extent;
}

void WindowApplication::CreateImageViews()
{
	swapchain_image_views.resize(swapchain_images.size());

	for(size_t i=0; i<swapchain_images.size(); i++)
	{
		swapchain_image_views[i] = engine->GetVkDevice().createImageView(
			vk::ImageViewCreateInfo()
				.setImage(swapchain_images[i])
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(swapchain_image_format)
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
	}
}

void WindowApplication::RecreateSwapchain()
{
	engine->GetVkDevice().waitIdle();

	CleanupSwapchain();
	CreateSwapchain();
	CreateImageViews();
}


void WindowApplication::CreateSemaphores()
{
	image_available_semaphore = engine->GetVkDevice().createSemaphore(vk::SemaphoreCreateInfo());
	render_finished_semaphore = engine->GetVkDevice().createSemaphore(vk::SemaphoreCreateInfo());
}

void WindowApplication::RunMainLoop()
{
	float delta_time = 0.0f;

	auto last_frame_time = std::chrono::high_resolution_clock::now();
	while(true)
	{
		if(glfwWindowShouldClose(window))
			break;

		glfwPollEvents();

		Update(delta_time);

		DrawAndPresentFrame();

		auto time = std::chrono::high_resolution_clock::now();
		delta_time = std::chrono::duration<float, std::ratio<1>>(time - last_frame_time).count();
		last_frame_time = time;
	}

	engine->GetVkDevice().waitIdle();
}

void WindowApplication::DrawAndPresentFrame()
{
	auto image_index_result = engine->GetVkDevice().acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), image_available_semaphore, vk::Fence() /*nullptr*/);

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

	DrawFrame(image_index);

	vk::Semaphore signal_semaphores[] = { render_finished_semaphore };

	auto present_result = engine->GetPresentQueue().presentKHR(vk::PresentInfoKHR()
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

	engine->GetPresentQueue().waitIdle();
}

void WindowApplication::CleanupSwapchain()
{
	auto device = engine->GetVkDevice();

	for(const auto &image_view : swapchain_image_views)
		device.destroyImageView(image_view);

	device.destroySwapchainKHR(swapchain);
}


void WindowApplication::Cleanup()
{
	auto device = engine->GetVkDevice();

	CleanupSwapchain();

	CleanupApplication();

	device.destroySemaphore(image_available_semaphore);
	device.destroySemaphore(render_finished_semaphore);

	engine->GetVkInstance().destroySurfaceKHR(surface);

	delete engine;

	glfwDestroyWindow(window);
    glfwTerminate();
}

void WindowApplication::OnWindowResized(GLFWwindow *window, int width, int height)
{
	if(width == 0 || height == 0)
		return;

	auto *app = reinterpret_cast<WindowApplication *>(glfwGetWindowUserPointer(window));
	app->RecreateSwapchain();
}