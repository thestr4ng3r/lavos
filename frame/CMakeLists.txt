
set(SOURCE_FILES
		include/window_application.h
		src/window_application.cpp)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(lavosframe ${SOURCE_FILES})