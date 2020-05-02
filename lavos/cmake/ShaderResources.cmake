
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

function(compile_spirv)
	cmake_parse_arguments(ARG "" "SPIRV_FILE;GLSL_FILE" "GLSLC_FLAGS" ${ARGN})
	execute_process(COMMAND "${Shaderc_GLSLC}" -x glsl ${ARG_GLSLC_FLAGS} -M "${ARG_GLSL_FILE}"
			OUTPUT_VARIABLE GLSL_MAKE_DEPS)
	parse_make_deps(GLSL_DEPS "${GLSL_MAKE_DEPS}")

	add_custom_command(OUTPUT "${spirv_file_abs}"
			COMMAND "${Shaderc_GLSLC}" ${ARG_GLSLC_FLAGS} -x glsl "${ARG_GLSL_FILE}" -o "${ARG_SPIRV_FILE}"
			MAIN_DEPENDENCY "${ARG_GLSL_FILE}"
			DEPENDS ${GLSL_DEPS})
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

	set(${spirv_files} "")
	foreach(glsl_file ${glsl_files})
		if(IS_ABSOLUTE "${glsl_file}")
			file(RELATIVE_PATH glsl_file "${ARG_BASE_DIR}" "${glsl_file}")
		endif()
		set(glsl_file_abs "${ARG_BASE_DIR}/${glsl_file}")

		get_filename_component(spirv_file_dir "${ARG_TARGET_DIR}/${glsl_file}" DIRECTORY)
		file(MAKE_DIRECTORY "${spirv_file_dir}")

		if(glsl_file MATCHES "(.+)\\.([vf]+)\\.shader")
			# compile stuff like "filename.vf.shader" to "filename.vert.spv" and "filename.frag.spv"
			# with defines SHADER_VERTEX or SHADER_FRAGMENT respectively

			set(file_base_name "${CMAKE_MATCH_1}")
			set(file_stage_chars "${CMAKE_MATCH_2}")

			set(STAGES
					v VERT vert
					f FRAG frag)

			foreach(i RANGE 0 3 3)
				list(GET STAGES "${i}+0" stage_char)
				math(EXPR i "${i}+1")
				list(GET STAGES "${i}+1" stage_define)
				math(EXPR i "${i}+1")
				list(GET STAGES "${i}+2" stage_name)
				if(NOT "${file_stage_chars}" MATCHES ".*${stage_char}.*")
					continue()
				endif()

				set(spirv_file_abs "${ARG_TARGET_DIR}/${file_base_name}.${stage_name}.spv")
				list(APPEND spirv_files "${spirv_file_abs}")
				compile_spirv(
						SPIRV_FILE "${spirv_file_abs}"
						GLSL_FILE "${glsl_file_abs}"
						GLSLC_FLAGS "-DSHADER_${stage_define}=1" "-fshader-stage=${stage_name}")
			endforeach()

		else()
			# otherwise just compile "filename.abc" to "filename.abc.spv"
			set(spirv_file_abs "${ARG_TARGET_DIR}/${glsl_file}.spv")
			list(APPEND spirv_files "${spirv_file_abs}")
			compile_spirv(
					SPIRV_FILE "${spirv_file_abs}"
					GLSL_FILE "${glsl_file_abs}")
		endif()
	endforeach()
	set("${ARG_VAR}" "${spirv_files}" PARENT_SCOPE)
endfunction()