
#include <android/log.h>
#include <android_native_app_glue.h>
#include <vulkan/vulkan.hpp>

#include <common.h>

#include <triangle_application.h>

void sample_main()
{
	TriangleApplication demo_app;

	try
	{
		demo_app.Run();
	}
	catch(const std::runtime_error &e)
	{
		LOGE("%s\n", e.what());
	}
}