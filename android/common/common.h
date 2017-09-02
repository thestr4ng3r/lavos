
#ifndef ANDROID_COMMON_H
#define ANDROID_COMMON_H

#include <android/log.h>
#include <android_native_app_glue.h>

#include <vector>
#include <string>

#define LOG_TAG "native-activity"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))

void AndroidSetApp(android_app *_app);
ANativeWindow *AndroidGetApplicationWindow();
void AndroidGetWindowSize(int32_t *width, int32_t *height);
std::vector<char> AndroidReadSPIRVShader(const std::string shader);

void sample_main();

#endif //ANDROID_COMMON_H
