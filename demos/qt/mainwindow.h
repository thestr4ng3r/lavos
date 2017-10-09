
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
		engine::Engine *engine;
		QVulkanWindow *window;

		engine::PhongMaterial *material;
		engine::Renderer *renderer;

		engine::AssetContainer *asset_container;

	public:
		MainWindowRenderer(engine::Engine *engine, QVulkanWindow *window)
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
		engine::Engine *engine;

	public:
		MainWindow(engine::Engine *engine, QWindow *parent = nullptr)
			: QVulkanWindow(parent), engine(engine) {}

		QVulkanWindowRenderer *createRenderer() override
		{
			return new MainWindowRenderer(engine, this);
		}

};

#endif //VULKAN_MAINWINDOW_H
