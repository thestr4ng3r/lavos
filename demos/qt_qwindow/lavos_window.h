
#ifndef LAVOS_LAVOS_WINDOW_H
#define LAVOS_LAVOS_WINDOW_H

#include <lavos/engine.h>
#include <lavos/material/phong_material.h>
#include <lavos/renderer.h>
#include <lavos/asset_container.h>
#include <lavos/swapchain.h>

#include <QWindow>
#include <QtGui/QVulkanWindow>

class LavosWindow: public QWindow
{
	Q_OBJECT

	public:
		class Renderer
		{
			public:
				virtual void InitializeSwapchainResources(LavosWindow *window) =0;
				virtual void ReleaseSwapchainResources() =0;
				virtual void Render(LavosWindow *window) =0;
		};

	private:
		lavos::Engine *engine;
		Renderer *renderer;

		bool vulkan_initialized;

		vk::SurfaceKHR surface;
		uint32_t present_queue_family_index;
		vk::Queue present_queue;
		lavos::Swapchain *swapchain;
		lavos::ManagedDepthRenderTarget *depth_render_target;

		vk::Semaphore image_available_semaphore;
		vk::Semaphore render_finished_semaphore;

		void RecreateSwapchain();
		void InitializeVulkan();
		void CleanupVulkan();

	protected:
		bool event(QEvent *event) override;
		void exposeEvent(QExposeEvent *ev) override;
		void resizeEvent(QResizeEvent *ev) override;

	public:
		LavosWindow(lavos::Engine *engine, Renderer *renderer, QWindow *parent = nullptr);

		void Render(lavos::Renderer *renderer);

		lavos::Engine *GetEngine() const 								{ return engine; }

		lavos::Swapchain *GetSwapchain() const							{ return swapchain; }
		lavos::ManagedDepthRenderTarget *GetDepthRenderTarget() const	{ return depth_render_target; }
};

#endif //VULKAN_MAINWINDOW_H
