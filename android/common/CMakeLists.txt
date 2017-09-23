

add_library(demo_android STATIC
    android_common.cpp
    android_common.h
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

target_include_directories(demo_android PRIVATE
                          ${ANDROID_NDK}/sources/android/native_app_glue
                          ${ANDROID_NDK}/sources/third_party/shaderc/include)

target_link_libraries(demo_android
    android
    log
    vulkan
    ${MODULE_NAME}
    demo_common)
