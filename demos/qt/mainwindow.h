
#ifndef VULKAN_MAINWINDOW_H
#define VULKAN_MAINWINDOW_H

#include <engine.h>
#include <material/phong_material.h>
#include <renderer.h>
#include <asset_container.h>

#include <QWindow>
#include <QtGui/QVulkanWindow>

class MainWindowRenderer: public QVulkanWindowRenderer
{
	private:
		lavos::Engine *engine;
		QVulkanWindow *window;

		lavos::PhongMaterial *material;
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
