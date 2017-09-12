if(MSVC)
    set(_common "/DWIN32 /D_WINDOWS /W4 /MP")
    set(_common_debug "/MT /ZI /Ob0 /Od /RTC1")
    set(_common_release "/DNDEBUG /MT /Ob2 /Os")
    set(_common_libraries "kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib")

    set(CMAKE_C_FLAGS "${_common}" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_DEBUG "${_common_debug}" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_RELEASE "${_common_release}" CACHE STRING "" FORCE)
    set(CMAKE_C_STANDARD_LIBRARIES "${_common_libraries}" CACHE STRING "" FORCE)

    set(CMAKE_CXX_FLAGS "${_common} /GR- /EHsc- /std:c++14" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "${_common_debug}" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "${_common_release}" CACHE STRING "" FORCE)
    set(CMAKE_CXX_STANDARD_LIBRARIES "${_common_libraries}" CACHE STRING "" FORCE)

    set(CMAKE_EXE_LINKER_FLAGS "/machine:x64" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO" CACHE STRING "" FORCE)

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(_common "-Wall -Wextra -Wpedantic")
    set(_common_debug "-g -O0")
    set(_common_release "-Os -DNDEBUG")
    set(_common_libraries "-ldl")

    set(CMAKE_C_FLAGS "-std=c11 ${_common}" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_DEBUG "${_common_debug}" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_RELEASE "${_common_release}" CACHE STRING "" FORCE)
    set(CMAKE_C_STANDARD_LIBRARIES "${_common_libraries}" CACHE STRING "" FORCE)

    set(CMAKE_CXX_FLAGS "-std=c++1z -fno-rtti -fno-exceptions ${_common}" CACHE STRING "" FORCE)
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "-stdlib=libc++ ${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
    endif()

    set(CMAKE_CXX_FLAGS_DEBUG "${_common_debug}" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "${_common_release}" CACHE STRING "" FORCE)
    set(CMAKE_CXX_STANDARD_LIBRARIES "${_common_libraries}" CACHE STRING "" FORCE)

    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        get_filename_component(_clang_bin ${CMAKE_CXX_COMPILER} DIRECTORY)
        get_filename_component(_clang_lib ${_clang_bin}/../lib REALPATH)
        set(_link_flags "-Wl,-rpath,${_clang_lib}")
        set(CMAKE_CXX_STANDARD_LIBRARIES "-lc++ -lc++abi -lc++experimental ${CMAKE_CXX_STANDARD_LIBRARIES}"
            CACHE STRING "" FORCE)
    else()
        set(CMAKE_CXX_STANDARD_LIBRARIES "-lstdc++fs ${CMAKE_CXX_STANDARD_LIBRARIES}"
            CACHE STRING "" FORCE)
    endif()

    set(CMAKE_EXE_LINKER_FLAGS "${_link_flags}" CACHE STRING "" FORCE)

else()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()
