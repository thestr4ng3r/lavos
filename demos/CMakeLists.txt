
option(LAVOS_BUILD_QT_DEMOS "Build Qt Demo" OFF)

add_subdirectory(gltf)
add_subdirectory(firstperson)
add_subdirectory(2d)
add_subdirectory(room)
add_subdirectory(point_cloud)

if(LAVOS_BUILD_QT_DEMOS)
	if(NOT LAVOS_BUILD_SHELL_QT)
		message(FATAL_ERROR "Lavos Qt Shell is required for the Qt demos. Either enable it using -DLAVOS_BUILD_SHELL_QT=ON or disable building the Qt demos.")
	endif()

	add_subdirectory(qt_qvulkanwindow)
	add_subdirectory(qt_shell)
endif()