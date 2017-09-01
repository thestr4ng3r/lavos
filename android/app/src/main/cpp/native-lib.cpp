
#include <android/log.h>
#include <android_native_app_glue.h>
#include <vulkan/vulkan.hpp>

#define LOG_TAG "native-activity"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))

extern "C" void android_main(struct android_app *state)
{
	//InitVulkan();

	auto extensions_available = vk::enumerateInstanceExtensionProperties();
	LOGI("Available Extensions:\n");
	for(const auto &extension : extensions_available)
		LOGI("\t%s\n", extension.extensionName);
}