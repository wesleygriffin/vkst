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

add_custom_command(OUTPUT loader.h loader.cc
    COMMAND ${PYTHON_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/vk/create_loader.py 
        ${VULKAN_INCLUDE_DIR}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/vk/create_loader.py
    COMMENT Creating loader
)

add_library(vk OBJECT
    ${CMAKE_CURRENT_BINARY_DIR}/loader.cc
    vk/result.cc
)
add_dependencies(vk plat turf shaderc)
target_include_directories(vk PUBLIC ${TURF_INCLUDE_DIR} ${GSL_INCLUDE_DIR} ${SHADERC_INCLUDE_DIR})

add_executable(st WIN32 st.cc renderer.cc $<TARGET_OBJECTS:plat>
    $<TARGET_OBJECTS:wsi> $<TARGET_OBJECTS:vk>
)
target_compile_definitions(st PRIVATE "PROJECT_DIR=\"${PROJECT_SOURCE_DIR}\"")
target_include_directories(st PRIVATE
    ${GLM_INCLUDE_DIR} ${SHADERC_INCLUDE_DIR} ${TURF_INCLUDE_DIR} ${GSL_INCLUDE_DIR}
)
target_link_libraries(st PRIVATE ${SHADERC_LIBRARY})
