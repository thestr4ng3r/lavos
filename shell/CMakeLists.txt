
if(LAVOS_BUILD_SHELL_GLFW)
    add_subdirectory(glfw)
endif()

if(LAVOS_BUILD_SHELL_QT)
	add_subdirectory(qt)
endif()