
#include <lavos/engine.h>
#include <lavos/component/directional_light_component.h>
#include <lavos/material/phong_material.h>
#include <lavos/renderer.h>
#include <lavos/asset_container.h>

#include "mainwindow.h"

#include <QPlatformSurfaceEvent>

void MainWindowRenderer::InitResources()
{
	material = new lavos::PhongMaterial(engine);

	asset_container = lavos::AssetContainer::LoadFromGLTF(engine, material, "data/gltftest.gltf");

	scene = asset_container->scenes[0];
	scene->SetAmbientLightIntensity(glm::vec3(0.3f, 0.3f, 0.3f));

	camera = scene->GetRootNode()->GetComponentInChildren<lavos::CameraComponent>();

	if(camera == nullptr)
	{
		lavos::Node *camera_node = new lavos::Node();
		scene->GetRootNode()->AddChild(camera_node);

		camera_node->AddComponent(new lavos::TransformComponent());

		camera_node->GetTransformComponent()->translation = glm::vec3(5.0f, 5.0f, 5.0f);
		camera_node->GetTransformComponent()->SetLookAt(glm::vec3(0.0f, 0.0f, 0.0f));

		camera = new lavos::CameraComponent();
		camera->SetNearClip(0.01f);
		camera_node->AddComponent(camera);
	}

	lavos::Node *light_node = new lavos::Node();
	scene->GetRootNode()->AddChild(light_node);

	light_node->AddComponent(new lavos::TransformComponent());
	light_node->GetTransformComponent()->SetLookAt(glm::vec3(-1.0f, -1.0f, -1.0f));

	lavos::DirectionalLightComponent *light = new lavos::DirectionalLightComponent();
	light_node->AddComponent(light);
}

void MainWindowRenderer::InitSwapchainResources()
{
	renderer = new lavos::Renderer(engine, window->GetSwapchain(), window->GetDepthRenderTarget());
	renderer->AddMaterial(material);

	renderer->SetScene(scene);
	renderer->SetCamera(camera);
}

void MainWindowRenderer::ReleaseResources()
{
	delete renderer;
	delete asset_container;
	delete material;
	delete engine;
}

void MainWindowRenderer::Render()
{
	window->Render(renderer);
}



MainWindow::MainWindow(lavos::Engine *engine, QWindow *parent)
		: QWindow(parent), engine(engine)
{
	vulkan_initialized = false;
}

void MainWindow::Initialize()
{
	surface = QVulkanInstance::surfaceForWindow(this);
	present_queue_family_index = static_cast<uint32_t>(engine->FindPresentQueueFamily(surface));
	present_queue = engine->GetVkDevice().getQueue(present_queue_family_index, 0);
	vk::Extent2D extent(static_cast<uint32_t>(width()), static_cast<uint32_t>(height()));
	swapchain = new lavos::Swapchain(engine, surface, present_queue_family_index, extent);
	depth_render_target = new lavos::ManagedDepthRenderTarget(engine, swapchain);

	image_available_semaphore = engine->GetVkDevice().createSemaphore(vk::SemaphoreCreateInfo());
	render_finished_semaphore = engine->GetVkDevice().createSemaphore(vk::SemaphoreCreateInfo());
}

void MainWindow::RecreateSwapchain()
{
	swapchain->Recreate();
}

void MainWindow::Render(lavos::Renderer *renderer)
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

bool MainWindow::event(QEvent *event)
{
	if(event->type() == QEvent::PlatformSurface)
	{
		QPlatformSurfaceEvent *surface_event = static_cast<QPlatformSurfaceEvent *>(event);
		if(surface_event->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed)
		{
			emit surfaceAboutToBeDestroyed();
		}
	}
	QWindow::event(event);
}

void MainWindow::exposeEvent(QExposeEvent *ev)
{
	if(!vulkan_initialized && isExposed())
	{
		Initialize();
		emit initializeSwapchain();
	}

	QWindow::exposeEvent(ev);
}

