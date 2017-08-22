include(ExternalProject)

ExternalProject_Add(turf
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/turf
    DOWNLOAD_COMMAND ""
    CMAKE_ARGS
        "-DTURF_PREFER_CPP11=TRUE"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
)
ExternalProject_Get_Property(turf INSTALL_DIR)
set(TURF_INCLUDE_DIR ${INSTALL_DIR}/include CACHE PATH "" FORCE)
set(TURF_LIBRARY ${INSTALL_DIR}/lib/turf${CMAKE_STATIC_LIBRARY_PREFIX} CACHE PATH "" FORCE)

set(GSL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gsl-lite/include CACHE PATH "" FORCE)

find_package(Vulkan REQUIRED)
find_package(PythonInterp 3 REQUIRED)

ExternalProject_Add(shaderc
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaderc
    DOWNLOAD_COMMAND ""
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy ../update_shaderc_sources.py <SOURCE_DIR>
    SOURCE_SUBDIR src
    INSTALL_DIR <PREFIX>
    CMAKE_ARGS
        "-DSHADERC_SKIP_TESTS=TRUE"
        "-DSHADERC_ENABLE_SHARED_CRT=TRUE"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
    BUILD_BYPRODUCTS
        <INSTALL_DIR>/include/shaderc/shaderc.hpp
        <INSTALL_DIR>/lib/shaderc_combined.lib
)
ExternalProject_Add_Step(shaderc DOWNLOAD_SOURCES
    COMMAND ${PYTHON_EXECUTABLE} update_shaderc_sources.py
    WORKING_DIRECTORY <SOURCE_DIR>
    COMMENT "Downloading shaderc sources"
    DEPENDEES PATCH DEPENDERS CONFIGURE
)
ExternalProject_Get_Property(shaderc INSTALL_DIR)
set(SHADERC_INCLUDE_DIR ${INSTALL_DIR}/include CACHE PATH "" FORCE)
set(SHADERC_LIBRARY ${INSTALL_DIR}/lib/shaderc_combined.lib CACHE PATH "" FORCE)

set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glm CACHE PATH "" FORCE)