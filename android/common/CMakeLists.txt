

add_library(common STATIC
    android_common.cpp
    android_common.h)

target_include_directories(common PRIVATE
                          ${ANDROID_NDK}/sources/android/native_app_glue
                          ${ANDROID_NDK}/sources/third_party/shaderc/include)

target_link_libraries(common
    android
    log
    vulkan
    native_app_glue)