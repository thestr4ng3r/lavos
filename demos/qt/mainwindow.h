
#ifndef VULKAN_MAINWINDOW_H
#define VULKAN_MAINWINDOW_H

#include <lavos/engine.h>
#include <lavos/material/phong_material.h>
#include <lavos/renderer.h>
#include <lavos/asset_container.h>

#include <QWindow>
#include <QtGui/QVulkanWindow>

class QVulkanWindowColorRenderTarget : public lavos::ColorRenderTarget
{
	private:
		QVulkanWindow * const vulkan_window;

	public:
		explicit QVulkanWindowColorRenderTarget(QVulkanWindow * const vulkan_window)
				: vulkan_window(vulkan_window) {}

		vk::Extent2D GetExtent() const override
		{
			auto size = vulkan_window->swapChainImageSize();
			return vk::Extent2D(static_cast<uint32_t>(size.width()),
								static_cast<uint32_t>(size.height()));
		}

		vk::Format GetFormat() const override
		{
			return vk::Format(vulkan_window->colorFormat());
		}

		std::vector<vk::ImageView> GetImageViews() const override
		{
			int count = vulkan_window->swapChainImageCount();
			std::vector<vk::ImageView> ret(static_cast<unsigned long>(count));
			for (int i=0; i<count; i++)
			{
				ret[i] = vulkan_window->swapChainImageView(i);
			}
			return ret;
		}

		void SwapchainChanged()
		{
			SignalChangedCallbacks();
		}
};

class QVulkanWindowDepthRenderTarget : public lavos::DepthRenderTarget
{
	private:
		QVulkanWindow * const vulkan_window;

	public:
		explicit QVulkanWindowDepthRenderTarget(QVulkanWindow * const vulkan_window)
				: vulkan_window(vulkan_window) {}

		vk::Extent2D GetExtent() const override
		{
			auto size = vulkan_window->swapChainImageSize();
			return vk::Extent2D(static_cast<uint32_t>(size.width()),
								static_cast<uint32_t>(size.height()));
		}

		vk::Format GetFormat() const override
		{
			return vk::Format(vulkan_window->depthStencilFormat());
		}

		vk::ImageView GetImageView() const override
		{
			return vulkan_window->depthStencilImageView();
		}
};

class MainWindowRenderer: public QVulkanWindowRenderer
{
	private:
		lavos::Engine *engine = nullptr;
		QVulkanWindow *window;

		lavos::PhongMaterial *material = nullptr;
		lavos::Scene *scene = nullptr;
		lavos::CameraComponent *camera = nullptr;

		QVulkanWindowColorRenderTarget *color_render_target = nullptr;
		QVulkanWindowDepthRenderTarget *depth_render_target = nullptr;
		lavos::Renderer *renderer = nullptr;

		lavos::AssetContainer *asset_container = nullptr;

	public:
		MainWindowRenderer(lavos::Engine *engine, QVulkanWindow *window)
			: engine(engine), window(window) {}

		void initResources() override;
		void initSwapChainResources() override;
		void releaseSwapChainResources() override;
		void releaseResources() override;

		void startNextFrame() override;
};

class MainWindow: public QVulkanWindow
{
	Q_OBJECT

	private:
		lavos::Engine *engine;

	public:
		MainWindow(lavos::Engine *engine, QWindow *parent = nullptr)
			: QVulkanWindow(parent), engine(engine) {}

		QVulkanWindowRenderer *createRenderer() override
		{
			return new MainWindowRenderer(engine, this);
		}

};

#endif //VULKAN_MAINWINDOW_H
