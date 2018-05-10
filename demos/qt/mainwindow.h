
#ifndef VULKAN_MAINWINDOW_H
#define VULKAN_MAINWINDOW_H

#include <lavos/engine.h>
#include <lavos/material/phong_material.h>
#include <lavos/renderer.h>
#include <lavos/asset_container.h>

#include <QWindow>
#include <QtGui/QVulkanWindow>

class QVulkanWindowRenderTarget : public lavos::ColorRenderTarget
{
	private:
		QVulkanWindow * const vulkan_window;

	public:
		QVulkanWindowRenderTarget(QVulkanWindow * const vulkan_window)
				: vulkan_window(vulkan_window) {}

		virtual vk::Extent2D GetExtent() const
		{
			auto size = vulkan_window->swapChainImageSize();
			return vk::Extent2D(static_cast<uint32_t>(size.width()),
								static_cast<uint32_t>(size.height()));
		}

		virtual vk::Format GetFormat() const
		{
			return vk::Format(vulkan_window->colorFormat());
		}

		virtual std::vector<vk::ImageView> GetImageViews() const
		{
			int count = vulkan_window->swapChainImageCount();
			std::vector<vk::ImageView> ret(static_cast<unsigned long>(count));
			for (int i=0; i<count; i++)
			{
				ret[i] = vulkan_window->swapChainImageView(i);
			}
		}

		void SwapchainChanged()
		{
			SignalChangedCallbacks();
		}
};

class MainWindowRenderer: public QVulkanWindowRenderer
{
	private:
		lavos::Engine *engine;
		QVulkanWindow *window;

		lavos::PhongMaterial *material;
		QVulkanWindowRenderTarget *render_target = nullptr;
		lavos::Renderer *renderer;

		lavos::AssetContainer *asset_container;

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
