cmake_minimum_required(VERSION 3.2)

include(CMakeParseArguments)

set(RESOURCE_GENERATOR_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

function(generate_resources)
	set(options GENERATE_HEADER GENERATE_SOURCE BINARY)
	set(one_value_args RELATIVE_PATH HEADER_FILE HEADER_FILE_RELATIVE_PATH SOURCE_FILE FUNCTION_PREFIX INCLUDE_GUARD TYPE)
	set(multi_value_args FILES)
	cmake_parse_arguments(RESOURCE_GENERATOR "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

	if(NOT RESOURCE_GENERATOR_FUNCTION_PREFIX)
		set(RESOURCE_GENERATOR_FUNCTION_PREFIX "resources")
	endif()

	if(NOT RESOURCE_GENERATOR_TYPE)
		if(RESOURCE_GENERATOR_BINARY)
			set(RESOURCE_GENERATOR_TYPE "unsigned char")
		else()
			set(RESOURCE_GENERATOR_TYPE "char")
		endif()
	endif()


	if(RESOURCE_GENERATOR_GENERATE_HEADER)
		message("Generating resource header ${RESOURCE_GENERATOR_HEADER_FILE}")

		if(NOT RESOURCE_GENERATOR_INCLUDE_GUARD)
			string(TOUPPER ${RESOURCE_GENERATOR_FUNCTION_PREFIX} RESOURCE_GENERATOR_INCLUDE_GUARD)
		endif()
		configure_file("${RESOURCE_GENERATOR_ROOT_DIR}/ResourceGenerator_header.h.in" ${RESOURCE_GENERATOR_HEADER_FILE})
	endif()




	if(RESOURCE_GENERATOR_GENERATE_SOURCE)
		message("Generating resource source ${RESOURCE_GENERATOR_SOURCE_FILE}")

		set(RESOURCE_GENERATOR_RESOURCES_SOURCE "")
		set(RESOURCE_GENERATOR_GET_FUNCTION_SOURCE "")
		set(RESOURCE_GENERATOR_RESOURCES_ALL " ")
		set(RESOURCE_GENERATOR_RESOURCES_ALL_NAMES " ")

		set(resource_index 0)

		foreach(file ${RESOURCE_GENERATOR_FILES})
			file(RELATIVE_PATH file_rel ${RESOURCE_GENERATOR_RELATIVE_PATH} ${file})

			if(RESOURCE_GENERATOR_BINARY)
				file(READ ${file} file_content_hex HEX)
				string(LENGTH "${file_content_hex}" file_content_length)
				set(char_index 0)
				set(file_content "")
				while(char_index LESS file_content_length)
					string(SUBSTRING "${file_content_hex}" ${char_index} 2 byte)
					math(EXPR char_index "${char_index}+2")
					set(file_content "${file_content}0x${byte}, ")
				endwhile()
				set(file_content "{ ${file_content}}")
				math(EXPR file_content_length "${file_content_length}/2")
			else()
				file(READ ${file} file_content)
				string(LENGTH "${file_content}" file_content_length)
				string(REPLACE "\"" "\\\"" file_content "${file_content}")
				string(REPLACE "\n" "\\n\"\n\t\"" file_content "${file_content}")
				set(file_content "\"${file_content}\"")
			endif()

			set(variable_name "resource_data_${resource_index}")

			set(RESOURCE_GENERATOR_RESOURCES_SOURCE "${RESOURCE_GENERATOR_RESOURCES_SOURCE}static const ${RESOURCE_GENERATOR_TYPE} ${variable_name}[] =\n\t${file_content};\n\n")
			set(RESOURCE_GENERATOR_GET_FUNCTION_SOURCE "${RESOURCE_GENERATOR_GET_FUNCTION_SOURCE}\tif(strcmp(filename, \"${file_rel}\") == 0)\n\t{\n\t\tif(size != 0)\n\t\t\t*size = ${file_content_length};\n\t\treturn ${variable_name};\n\t}\n")
			set(RESOURCE_GENERATOR_RESOURCES_ALL "${RESOURCE_GENERATOR_RESOURCES_ALL}${variable_name}, ")
			set(RESOURCE_GENERATOR_RESOURCES_ALL_NAMES "${RESOURCE_GENERATOR_RESOURCES_ALL_NAMES}\"${file_rel}\", ")

			math(EXPR resource_index "${resource_index}+1")
		endforeach()

		set(RESOURCE_GENERATOR_RESOURCES_COUNT ${resource_index})

		file(RELATIVE_PATH RESOURCE_GENERATOR_HEADER_FILE_RELATIVE ${RESOURCE_GENERATOR_HEADER_FILE_RELATIVE_PATH} ${RESOURCE_GENERATOR_HEADER_FILE})

		configure_file("${RESOURCE_GENERATOR_ROOT_DIR}/ResourceGenerator_source.cpp.in" ${RESOURCE_GENERATOR_SOURCE_FILE})
	endif()

endfunction(generate_resources)