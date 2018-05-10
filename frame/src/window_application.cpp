
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

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	vk::Extent2D extent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

	present_queue_family_index = static_cast<uint32_t>(engine->FindPresentQueueFamily(surface));
	present_queue = engine->GetVkDevice().getQueue(present_queue_family_index, 0);
	swapchain = new lavos::Swapchain(engine, surface, present_queue_family_index, extent);
	depth_render_target = new lavos::ManagedDepthRenderTarget(engine, swapchain);

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

void WindowApplication::RecreateSwapchain()
{
	swapchain->Recreate();
}


void WindowApplication::CreateSemaphores()
{
	image_available_semaphore = engine->GetVkDevice().createSemaphore(vk::SemaphoreCreateInfo());
	render_finished_semaphore = engine->GetVkDevice().createSemaphore(vk::SemaphoreCreateInfo());
}


void WindowApplication::BeginFrame()
{
	if(last_frame_time == std::chrono::high_resolution_clock::time_point())
		std::chrono::high_resolution_clock::now();
}

void WindowApplication::Update()
{
	glfwPollEvents();
}

void WindowApplication::Render(lavos::Renderer *renderer)
{
	auto image_index_result = engine->GetVkDevice().acquireNextImageKHR(swapchain->GetSwapchain(),
																		std::numeric_limits<uint64_t>::max(),
																		image_available_semaphore,
																		vk::Fence() /*nullptr*/);

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

	renderer->DrawFrame(image_index,
						{ image_available_semaphore },
						{ vk::PipelineStageFlagBits::eColorAttachmentOutput },
						{ render_finished_semaphore });

	vk::Semaphore signal_semaphores[] = { render_finished_semaphore };

	vk::SwapchainKHR vk_swapchain = swapchain->GetSwapchain();
	vk::Result present_result;
	try
	{
		present_result = present_queue.presentKHR(vk::PresentInfoKHR()
														  .setWaitSemaphoreCount(1)
														  .setPWaitSemaphores(signal_semaphores)
														  .setSwapchainCount(1)
														  .setPSwapchains(&vk_swapchain)
														  .setPImageIndices(&image_index));
	}
	catch(vk::OutOfDateKHRError)
	{
        RecreateSwapchain();
        present_result = vk::Result::eSuccess;
	}

	if(present_result == vk::Result::eSuboptimalKHR)
	{
		RecreateSwapchain();
	}
	else if(present_result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	present_queue.waitIdle();
}


void WindowApplication::EndFrame()
{
	auto time = std::chrono::high_resolution_clock::now();
	delta_time = std::chrono::duration<float, std::ratio<1>>(time - last_frame_time).count();
	last_frame_time = time;

	engine->GetVkDevice().waitIdle();
}


void WindowApplication::Cleanup()
{
	auto device = engine->GetVkDevice();

	delete swapchain;

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
	app->OnWindowResized(width, height);
}

void WindowApplication::OnWindowResized(int width, int height)
{
	swapchain->Resize(vk::Extent2D(static_cast<uint32_t>(width), static_cast<uint32_t>(height)));
}
