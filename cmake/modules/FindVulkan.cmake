# Find Vulkan
#
# VULKAN_INCLUDE_DIR
# VULKAN_LIBRARY
# VULKAN_FOUND
# GLSLANGVALIDATOR_EXECUTABLE
# SHADERC_LIBRARY

if (WIN32)
    find_path(VULKAN_INCLUDE_DIR NAMES vulkan/vulkan.h HINTS
        "$ENV{VULKAN_SDK}/Include"
        "$ENV{VK_SDK_PATH}/Include")
    if (CMAKE_CL_64)
        find_library(VULKAN_LIBRARY NAMES vulkan-1 HINTS
            "$ENV{VULKAN_SDK}/Lib"
            "$ENV{VK_SDK_PATH}/Lib")
        find_program(GLSLANGVALIDATOR_EXECUTABLE NAMES glslangValidator.exe HINTS
            "$ENV{VULKAN_SDK}/Bin"
            "$ENV{VK_SDK_PATH}/Bin")
        find_library(SHADERC_LIBRARY NAMES shaderc_combined HINTS
            "$ENV{VULKAN_SDK}/Lib"
            "$ENV{VK_SDK_PATH}/Lib")
    else()
        find_library(VULKAN_LIBRARY NAMES vulkan-1 HINTS
            "$ENV{VULKAN_SDK}/Lib32"
            "$ENV{VK_SDK_PATH}/Lib32")
        find_program(GLSLANGVALIDATOR_EXECUTABLE NAMES glslangValidator.exe HINTS
            "$ENV{VULKAN_SDK}/Bin32"
            "$ENV{VK_SDK_PATH}/Bin32")
        find_library(SHADERC_LIBRARY NAMES shaderc_combined HINTS
            "$ENV{VULKAN_SDK}/Lib32"
            "$ENV{VK_SDK_PATH}/Lib32")
    endif()
else()
    find_path(VULKAN_INCLUDE_DIR NAMES vulkan/vulkan.h HINTS
        "$ENV{VULKAN_SDK}/include")
    find_library(VULKAN_LIBRARY NAMES vulkan HINTS
        "$ENV{VULKAN_SDK}/lib")
    find_program(GLSLANGVALIDATOR_EXECUTABLE NAMES glslangValidator HINTS
        "$ENV{VULKAN_SDK}/bin")
    find_library(SHADERC_LIBRARY NAMES shaderc HINTS
        "$ENV{VULKAN_SDK}/lib")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vulkan DEFAULT_MSG VULKAN_LIBRARY
    VULKAN_INCLUDE_DIR GLSLANGVALIDATOR_EXECUTABLE SHADERC_LIBRARY)

mark_as_advanced(VULKAN_INCLUDE_DIR VULKAN_LIBRARY
    GLSLANGVALIDATOR_EXECUTABLE SHADERC_LIBRARY)
