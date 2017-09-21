cmake_minimum_required(VERSION 3.4.1)

add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)

add_library(native_lib SHARED)

set_property(TARGET native_lib APPEND_STRING PROPERTY LINK_FLAGS "-u ANativeActivity_onCreate")


add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

target_include_directories(native_lib PRIVATE
                          ${ANDROID_NDK}/sources/third_party/vulkan/src/include
                          ${CMAKE_SOURCE_DIR}/android/vulkan-hpp
                          ${ANDROID_NDK}/sources/third_party/shaderc/include
                          ${CMAKE_SOURCE_DIR}/demos/triangle)

target_link_libraries(native_lib
                          android
                          log
                          native_app_glue
                          vulkan
                          common
                          ${MODULE_NAME}
                          demo_common)