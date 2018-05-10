
option(BUILD_QT_DEMO "Build Qt Demo" OFF)

add_subdirectory(gltf)
add_subdirectory(firstperson)
add_subdirectory(2d)
add_subdirectory(room)

if(BUILD_QT_DEMO)
	add_subdirectory(qt_qvulkanwindow)
endif()