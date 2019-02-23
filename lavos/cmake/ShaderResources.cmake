
include(ResourceGenerator)

find_package(Shaderc REQUIRED)

function(parse_make_deps OUTPUT_VAR MAKE_DEPS)
	if("${MAKE_DEPS}" MATCHES "^[^:]+:*(.*)$")
		string(STRIP "${CMAKE_MATCH_1}" DEPS)
		string(REPLACE " " ";" DEPS "${DEPS}")
		set("${OUTPUT_VAR}" "${DEPS}" PARENT_SCOPE)
	else()
		set("${OUTPUT_VAR}" "" PARENT_SCOPE)
	endif()
endfunction()

function(add_spirv_shaders)
	cmake_parse_arguments(ARG "" "TARGET_DIR;BASE_DIR;VAR" "" ${ARGN})
	if(NOT ARG_TARGET_DIR)
		message(FATAL_ERROR "add_spirv_shaders: No TARGET_DIR given.")
	endif()
	if(NOT ARG_VAR)
		message(FATAL_ERROR "add_spirv_shaders: No VAR given.")
	endif()
	if(NOT ARG_BASE_DIR)
		message("BASE_DIR not given")
		set(ARG_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
	endif()
	set(glsl_files "${ARG_UNPARSED_ARGUMENTS}")

	message("BASE_DIR=${ARG_BASE_DIR}")
	set(${spirv_files} "")
	foreach(glsl_file ${glsl_files})
		message("File ${glsl_file}")
		if(IS_ABSOLUTE "${glsl_file}")
			file(RELATIVE_PATH glsl_file "${ARG_BASE_DIR}" "${glsl_file}")
		endif()
		message("  Relative path: ${glsl_file}")
		set(spirv_file_abs "${ARG_TARGET_DIR}/${glsl_file}.spv")
		set(glsl_file_abs "${ARG_BASE_DIR}/${glsl_file}")
		list(APPEND spirv_files "${spirv_file_abs}")

		get_filename_component(spirv_file_dir "${spirv_file_abs}" DIRECTORY)
		file(MAKE_DIRECTORY "${spirv_file_dir}")

		execute_process(COMMAND "${Shaderc_GLSLC}" -M "${glsl_file_abs}"
				OUTPUT_VARIABLE GLSL_MAKE_DEPS)
		parse_make_deps(GLSL_DEPS "${GLSL_MAKE_DEPS}")

		add_custom_command(OUTPUT "${spirv_file_abs}"
				COMMAND "${Shaderc_GLSLC}" "${glsl_file_abs}" -o "${spirv_file_abs}"
				MAIN_DEPENDENCY "${glsl_file_abs}"
				DEPENDS ${GLSL_DEPS})
	endforeach()
	set("${ARG_VAR}" "${spirv_files}" PARENT_SCOPE)
endfunction()