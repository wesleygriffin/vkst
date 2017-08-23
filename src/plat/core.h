#ifndef VKST_PLAT_CORE_H
#define VKST_PLAT_CORE_H

#include <turf/c/core.h>

#ifndef __has_cpp_attribute
#  define PLAT_HAS_CPP_ATTRIBUTE(x) 0
#else
#  define PLAT_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#endif

#if TURF_COMPILER_MSVC

#define PLAT_MARK_UNREACHABLE __assume(0)

#define PLAT_PUSH_WARNING __pragma(warning(push))
#define PLAT_POP_WARNING __pragma(warning(pop))

#define PLAT_GCC_DISABLE_WARNING(name)
#define PLAT_MSVC_DISABLE_WARNING(num) __pragma(warning(disable : num))

#if PLAT_HAS_CPP_ATTRIBUTE(noreturn)
#  define PLAT_NORETURN [[noreturn]]
#else
#  define PLAT_NORETURN __declspec(noreturn)
#endif

#if PLAT_HAS_CPP_ATTRIBUTE(fallthrough)
#  define PLAT_FALLTHROUGH [[fallthrough]]
#else
#  define PLAT_FALLTHROUGH
#endif

#else // TURF_COMPILER_MSVC

#define PLAT_MARK_UNREACHABLE __builtin_unreachable()

#define PLAT_PUSH_WARNING _Pragma("GCC diagnostic push")
#define PLAT_POP_WARNING _Pragma("GCC diagnostic pop")

#define PLAT_GCC_DISABLE_WARNING(name)                                         \
  _Pragma(PLAT_STRINGIFY(GCC diagnostic ignored PLAT_STRINGIFY2(-W##name)))
#define PLAT_MSVC_DISABLE_WARNING(num)

#if PLAT_HAS_CPP_ATTRIBUTE(fallthrough)
#  define PLAT_FALLTHROUGH [[fallthrough]]
#else
#  define PLAT_FALLTHROUGH
#endif

#endif // TURF_COMPILER_MSVC

 #endif // VKST_PLAT_CORE_H