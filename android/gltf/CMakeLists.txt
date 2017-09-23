cmake_minimum_required(VERSION 3.4.1)

add_library(native_lib SHARED ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
set_property(TARGET native_lib APPEND_STRING PROPERTY LINK_FLAGS "-u ANativeActivity_onCreate")

target_link_libraries(native_lib
                          android
                          log
                          vulkan
                          common
                          ${MODULE_NAME}
                          demo_common)