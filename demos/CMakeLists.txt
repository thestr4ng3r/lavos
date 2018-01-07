
option(BUILD_QT_DEMO "Build Qt Demo" OFF)

add_subdirectory(gltf)
add_subdirectory(firstperson)

if(BUILD_QT_DEMO)
	add_subdirectory(qt)
endif()