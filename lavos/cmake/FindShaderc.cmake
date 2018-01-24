# FindShaderc
# -------
#
# This will will define the following variables:
#
# SHADERC_FOUND    - true if Shaderc has been found
# SHADERC_GLSLC    - the glslc executable

find_program(Shaderc_GLSLC glslc)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Shaderc REQUIRED_VARS Shaderc_GLSLC)