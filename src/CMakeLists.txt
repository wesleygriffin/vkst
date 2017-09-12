set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(CheckIncludeFileCXX)
include(CheckCXXSourceCompiles)

CHECK_INCLUDE_FILE_CXX("filesystem" PLAT_STD_FILESYSTEM)
if(PLAT_STD_FILESYSTEM)
    CHECK_CXX_SOURCE_COMPILES("#include <filesystem>\n\
int main() { std::filesystem::path p; }" PLAT_STD_FILESYSTEM_NS)
endif()

CHECK_INCLUDE_FILE_CXX("experimental/filesystem" PLAT_EXP_FILESYSTEM)
if(PLAT_EXP_FILESYSTEM)
    CHECK_CXX_SOURCE_COMPILES("#include <experimental/filesystem>\n\
int main() { std::experimental::filesystem::path p; }" PLAT_EXP_FILESYSTEM_NS)
endif()

configure_file(plat/plat_config.h.in plat/plat_config.h)

add_library(plat OBJECT
    plat/file_handle.cc
    plat/file_io.cc
    plat/fs_notify_linux.cc
    plat/fs_notify_win32.cc
    plat/log.cc
)
add_dependencies(plat turf)
target_include_directories(plat PUBLIC ${TURF_INCLUDE_DIR} ${GSL_INCLUDE_DIR})

add_library(wsi OBJECT
    wsi/input.cc
    wsi/window_win32.cc
    wsi/window_xlib.cc
)
add_dependencies(wsi plat turf)
target_include_directories(wsi PUBLIC ${TURF_INCLUDE_DIR} ${GSL_INCLUDE_DIR})

add_library(vk OBJECT
    vk/result.cc
)
add_dependencies(vk plat turf)
target_include_directories(vk PUBLIC
    ${VULKAN_INCLUDE_DIR} ${TURF_INCLUDE_DIR} ${GSL_INCLUDE_DIR})

add_executable(st WIN32 st.cc renderer.cc
    $<TARGET_OBJECTS:plat> $<TARGET_OBJECTS:wsi> $<TARGET_OBJECTS:vk>)
target_compile_definitions(st PRIVATE "PROJECT_DIR=\"${PROJECT_SOURCE_DIR}\"")
target_include_directories(st PRIVATE
    ${GLM_INCLUDE_DIR} ${VULKAN_INCLUDE_DIR} ${TURF_INCLUDE_DIR} ${GSL_INCLUDE_DIR})
target_link_libraries(st ${SHADERC_LIBRARY} ${VULKAN_LIBRARY})

add_executable(vkinfo WIN32 vkinfo.cc
    $<TARGET_OBJECTS:plat> $<TARGET_OBJECTS:wsi> $<TARGET_OBJECTS:vk>)
target_include_directories(vkinfo PRIVATE
    ${GLM_INCLUDE_DIR} ${VULKAN_INCLUDE_DIR} ${TURF_INCLUDE_DIR} ${GSL_INCLUDE_DIR})
target_link_libraries(vkinfo ${SHADERC_LIBRARY} ${VULKAN_LIBRARY})
