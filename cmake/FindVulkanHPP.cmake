# FindVulkanHPP
# ----------
#
# This module defines the following variables:
#
#   VulkanHPP_FOUND          - True if vulkan.hpp was found
#   VulkanHPP_INCLUDE_DIRS   - include directories for vulkan.hpp

find_path(VulkanHPP_INCLUDE_DIRS
        NAMES vulkan/vulkan.hpp
        PATHS
        "$ENV{VULKAN_SDK}/Include"
        "${VULKAN_INCLUDE_DIRS}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VulkanHPP DEFAULT_MSG VulkanHPP_INCLUDE_DIRS)
