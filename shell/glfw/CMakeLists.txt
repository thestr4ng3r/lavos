
set(SOURCE_FILES
        include/window_application.h
        src/window_application.cpp)

add_library(lavos_shell_glfw ${SOURCE_FILES})
target_include_directories(lavos_shell_glfw PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(lavos_shell_glfw PUBLIC lavos)

find_package(glfw3 3.3 REQUIRED)
target_link_libraries(lavos_shell_glfw PUBLIC glfw)