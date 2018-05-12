
#include <lavos/engine.h>
#include <lavos_window.h>

#include <QApplication>
#include <QVulkanInstance>

#include "lavos_renderer.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	lavos::Engine::CreateInfo engine_create_info;
	engine_create_info.app_info = "Qt Demo";

	engine_create_info.enable_validation_layers = true;
	engine_create_info.enable_anisotropy = true;

	engine_create_info.required_instance_extensions = {
		"VK_KHR_surface",
		"VK_KHR_xcb_surface"
	}; // TODO: do not hardcode these, make dynamic based on environment

	engine_create_info.required_device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	lavos::Engine *engine = new lavos::Engine(engine_create_info);
	engine->InitializeWithPhysicalDeviceIndex(0);

	QVulkanInstance inst;
	inst.setVkInstance(engine->GetVkInstance());

	if(!inst.create())
	{
		qFatal("Failed to create vulkan instance: %d", inst.errorCode());
	}

	LavosWindowRenderer renderer(engine);

	lavos::shell::qt::LavosWindow window(engine, &renderer);
	window.setVulkanInstance(&inst);
	window.setWidth(640);
	window.setHeight(480);
	window.show();

	return app.exec();
}