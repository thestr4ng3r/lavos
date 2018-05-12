
#include <lavos/engine.h>
#include <platform.h>

#include <QApplication>
#include <QVulkanInstance>

#include "mainwindow.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	lavos::Engine::CreateInfo engine_create_info;
	engine_create_info.app_info = "Qt Demo";

	engine_create_info.enable_validation_layers = true;
	engine_create_info.enable_anisotropy = false;

	engine_create_info.required_instance_extensions = lavos::shell::qt::GetSurfaceExtensionsForPlatform();

	lavos::Engine *engine = new lavos::Engine(engine_create_info);

	QVulkanInstance inst;
	inst.setVkInstance(engine->GetVkInstance());

	if(!inst.create())
	{
		qFatal("Failed to create vulkan instance: %d", inst.errorCode());
	}

	MainWindow window(engine);
	window.setVulkanInstance(&inst);
	window.setWidth(640);
	window.setHeight(480);
	window.show();

	return app.exec();
}