
#include <lavos/engine.h>
#include <lavos/component/directional_light_component.h>
#include <lavos/material/phong_material.h>
#include <lavos/renderer.h>
#include <lavos/asset_container.h>

#include "lavos_window.h"

#include <QPlatformSurfaceEvent>


using namespace lavos::shell::qt;


LavosWindow::LavosWindow(lavos::Engine *engine, Renderer *renderer, QWindow *parent)
		: QWindow(parent), engine(engine), renderer(renderer)
{
	vulkan_initialized = false;
	setSurfaceType(VulkanSurface);
}

LavosWindow::~LavosWindow()
{
	CleanupVulkan();
}

void LavosWindow::InitializeVulkan()
{
	surface = QVulkanInstance::surfaceForWindow(this);
	present_queue_family_index = static_cast<uint32_t>(engine->FindPresentQueueFamily(surface));
	present_queue = engine->GetVkDevice().getQueue(present_queue_family_index, 0);
	vk::Extent2D extent(static_cast<uint32_t>(width()), static_cast<uint32_t>(height()));
	swapchain = new lavos::Swapchain(engine, surface, present_queue_family_index, extent);
	depth_render_target = new lavos::ManagedDepthRenderTarget(engine, swapchain);

	image_available_semaphore = engine->GetVkDevice().createSemaphore(vk::SemaphoreCreateInfo());
	render_finished_semaphore = engine->GetVkDevice().createSemaphore(vk::SemaphoreCreateInfo());

	vulkan_initialized = true;
}

void LavosWindow::CleanupVulkan()
{
	auto device = engine->GetVkDevice();

	delete swapchain;
	swapchain = nullptr;
	delete depth_render_target;
	depth_render_target = nullptr;

	if(image_available_semaphore)
	{
		device.destroySemaphore(image_available_semaphore);
		image_available_semaphore = nullptr;
	}

	if(render_finished_semaphore)
	{
		device.destroySemaphore(render_finished_semaphore);
		render_finished_semaphore = nullptr;
	}

	vulkan_initialized = false;
}

void LavosWindow::RecreateSwapchain()
{
	swapchain->Recreate();
	requestUpdate();
}

void LavosWindow::Render(lavos::Renderer *renderer)
{
	if (!isExposed())
	{
		return;
	}

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

	vulkanInstance()->presentQueued(this);
	present_queue.waitIdle();
}

bool LavosWindow::event(QEvent *event)
{
	switch(event->type())
	{
		case QEvent::PlatformSurface:
		{
			QPlatformSurfaceEvent *surface_event = static_cast<QPlatformSurfaceEvent *>(event);
			if(surface_event->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed)
			{
				renderer->ReleaseSwapchainResources();
				CleanupVulkan();
			}
			break;
		}

		case QEvent::UpdateRequest:
			renderer->Render(this);
			break;

		default:
			break;
	}

	return QWindow::event(event);
}

void LavosWindow::exposeEvent(QExposeEvent *ev)
{
	if(isExposed())
	{
		if(!vulkan_initialized)
		{
			InitializeVulkan();
			renderer->InitializeSwapchainResources(this);
		}
		requestUpdate();
	}

	QWindow::exposeEvent(ev);
}

void LavosWindow::resizeEvent(QResizeEvent *)
{
}
