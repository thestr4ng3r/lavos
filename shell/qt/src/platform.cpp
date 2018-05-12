
#include "platform.h"

#include <QGuiApplication>

std::set<std::string> lavos::shell::qt::GetSurfaceExtensionsForPlatform()
{
	// Qt already has all this, but it is not publicly exposed :(

	const auto platform = QGuiApplication::platformName();

	if(platform == "xcb")
	{
		return {
			"VK_KHR_surface",
			"VK_KHR_xcb_surface"
		};
	}
	else if(platform == "wayland")
	{
		return {
			"VK_KHR_surface",
			"VK_KHR_wayland_surface"
		};
	}
	else if(platform == "windows")
	{
		return {
			"VK_KHR_surface",
			"VK_KHR_win32_surface"
		};
	}
	else if(platform == "cocoa")
	{
		return {
			"VK_KHR_surface",
			"VK_MVK_macos_surface"
		};
	}
}