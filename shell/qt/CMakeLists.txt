
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)
find_package(Qt5 5.10 COMPONENTS Core Gui REQUIRED)

set(SOURCE_FILES
        include/lavos_window.h
        src/lavos_window.cpp)

add_library(lavos_shell_qt ${SOURCE_FILES})
target_include_directories(lavos_shell_qt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(lavos_shell_qt
		Qt5::Core Qt5::Gui
		lavos)