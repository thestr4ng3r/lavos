
#ifndef VULKAN_MAINWINDOW_H
#define VULKAN_MAINWINDOW_H

#include <lavos/engine.h>
#include <lavos/material/phong_material.h>
#include <lavos/renderer.h>
#include <lavos/asset_container.h>
#include <lavos/swapchain.h>

#include <QWindow>
#include <QtGui/QVulkanWindow>

class MainWindow;

class MainWindowRenderer
{
	private:
		lavos::Engine *engine = nullptr;
		MainWindow *window;

		lavos::PhongMaterial *material = nullptr;
		lavos::Scene *scene = nullptr;
		lavos::CameraComponent *camera = nullptr;

		lavos::Renderer *renderer = nullptr;

		lavos::AssetContainer *asset_container = nullptr;

	public:
		MainWindowRenderer(lavos::Engine *engine, MainWindow *window)
			: engine(engine), window(window) {}

		void InitResources();
		void InitSwapchainResources();
		void ReleaseResources();

		void Render();
};

class MainWindow: public QWindow
{
	Q_OBJECT

	private:
		lavos::Engine *engine;

		bool vulkan_initialized;

		vk::SurfaceKHR surface;
		uint32_t present_queue_family_index;
		vk::Queue present_queue;
		lavos::Swapchain *swapchain;
		lavos::ManagedDepthRenderTarget *depth_render_target;

		vk::Semaphore image_available_semaphore;
		vk::Semaphore render_finished_semaphore;

		void RecreateSwapchain();

	protected:
		bool event(QEvent *event);
		void exposeEvent(QExposeEvent *ev) override;

	public:
		MainWindow(lavos::Engine *engine, QWindow *parent = nullptr);

		void Initialize();
		void Render(lavos::Renderer *renderer);

		lavos::Swapchain *GetSwapchain() const							{ return swapchain; }
		lavos::ManagedDepthRenderTarget *GetDepthRenderTarget() const	{ return depth_render_target; }

	signals:
		void initializeSwapchain();
		void surfaceAboutToBeDestroyed();
};

#endif //VULKAN_MAINWINDOW_H
