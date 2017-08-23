#!/usr/bin/env python3
import argparse
import os
import re
import sys

argv = sys.argv[1:]
argc = len(argv)

if argc != 1:
    print("invalid command line")
    sys.exit(1)

vulkan_dir = argv[0]
if not os.path.exists(vulkan_dir):
    print("{} does not exist".format(vulkan_dir))
    sys.exit(1)

vulkan_fn = os.path.join(vulkan_dir, "vulkan", "vulkan.h")
if not os.path.exists(vulkan_fn):
    print("{} does not exist".format(vulkan_fn))
    sys.exit(1)

platform_fn = os.path.join(vulkan_dir, "vulkan", "vk_platform.h")
if not os.path.exists(platform_fn):
    print("{} does not exist".format(platform_fn))
    sys.exit(1)

vulkan_h = open(vulkan_fn, 'r').read()
platform_h = open(platform_fn, 'r').read()

extension_names = []
for name in re.findall("#define\s+.*?_EXTENSION_NAME\s+\"VK_(.*?)\"", vulkan_h):
    extension_names.append(name)
extension_names = sorted(extension_names)

all_funcs = []
platform_funcs = {
    "": []
}

platform = None

function_re = re.compile(r"VKAPI_PTR\s+\*PFN_(\w+)")
platform_begin_re = re.compile(r"#ifdef\s+(VK_USE_PLATFORM\w+)")
platform_end_re = re.compile(r"#endif\s+/\*\s*(\w+)\s*\*/")

for line in vulkan_h.splitlines():
    platform_m = platform_begin_re.search(line)
    if platform_m:
        platform = platform_m.group(1)
        if platform not in platform_funcs:
            platform_funcs[platform] = []

    platform_m = platform_end_re.search(line)
    if platform_m and platform == platform_m.group(1):
        platform = None

    function_m = function_re.search(line)
    if function_m:
        func = function_m.group(1)

        if func not in all_funcs:
            all_funcs.append(func)

            if platform is None:
                platform_funcs[""].append(func)
            else:
                platform_funcs[platform].append(func)

def get_copyright(source):
    longest = ""
    copyright_re = re.compile("copyright", re.I)

    for comment in re.findall("\/\*[\s\S]*?\*\/", vulkan_h, re.M):
        if copyright_re.search(comment) and len(comment) > len(longest):
            longest = comment

    if not longest:
        return None

    return longest.strip() + "\n\n"

vulkan_h_copyright = get_copyright(vulkan_h)
platform_h_copyright = get_copyright(platform_h)

if vulkan_h_copyright == platform_h_copyright:
    platform_h_copyright = None

def process_source(source):
    source = re.compile("\s*?#\s*?ifdef\s+__cplusplus.*?#endif.*?$",
        re.S | re.I | re.M).sub("", source)
    source = re.compile(r"^\s*?#\s*?ifndef VK_NO_PROTOTYPES.*?^#\s*endif",
        re.S | re.M).sub("", source)
    source = re.compile("\/\*[\s\S]*?\*\/").sub("", source)
    source = re.compile("^//.*?\r?\n", re.M).sub("", source)
    source = re.compile("^\s+$", re.M).sub("", source)
    source = re.compile(r"^(\r?\n)\s*?(\r?\n\s*?)+^", re.M).sub(r"\1", source)
    return source.strip();

platform_h = process_source(platform_h)

vulkan_h = process_source(vulkan_h)
vulkan_h = re.compile(r"^\s*#\s*include\s+\"vk_platform.h\"",
    re.I | re.M).sub(platform_h, vulkan_h)
vulkan_h = process_source(vulkan_h)

with open("loader.h", "wb") as f:
    if vulkan_h_copyright:
        f.write(vulkan_h_copyright.encode("utf-8"))
    if platform_h_copyright:
        f.write(platform_h_copyright.encode("utf-8"))

    f.write(br'''#ifndef VK_LOADER_H
#define VK_LOADER_H
#include <system_error>

''')

    f.write(vulkan_h.encode("utf-8"))
    f.write(b"\n")

    f.write(br'''

namespace vk {

enum class loader_result {
    success = 0,
    no_library = 1,
    no_function = 2,
}; // enum class loader_result

class loader_result_category_impl : public std::error_category {
public:
  virtual char const* name() const noexcept override {
    return "vk::loader_result";
  }

  virtual std::string message(int ev) const override;
}; // class loader_result_category_impl

std::error_category const& loader_result_category();

inline std::error_code make_error_code(loader_result e) noexcept {
  return {static_cast<int>(e), loader_result_category()};
}

std::error_code init_loader() noexcept;
bool instance_extension_present(char const* name);
void load_instance_procs(VkInstance instance) noexcept;
bool device_extension_present(VkPhysicalDevice device, char const* name);
void load_device_procs(VkDevice device) noexcept;
void deinit_loader() noexcept;

} // namespace vk

''')

    lines = []
    lines.append("// Functions")

    for platform in sorted(platform_funcs):
        if platform:
            lines.append("#ifdef " + platform)
        for func in sorted(platform_funcs[platform]):
            lines.append("extern PFN_{0} _{0};".format(func))
        if platform:
            lines.append("#endif // " + platform)
        lines.append("")

    for platform in sorted(platform_funcs):
        if platform:
            lines.append("// {0}".format(platform))
            for func in sorted(platform_funcs[platform]):
                lines.append("#define {0} nullptr".format(func))
            lines.append("")
        else:
            for func in sorted(platform_funcs[platform]):
                lines.append("#define {0} _{0}".format(func))

    for platform in sorted(platform_funcs):
        if platform:
            lines.append("#ifdef " + platform)
            for func in sorted(platform_funcs[platform]):
                lines.append("#undef {0}".format(func))
            for func in sorted(platform_funcs[platform]):
                lines.append("#define {0} _{0}".format(func))
            lines.append("#endif // " + platform)
            lines.append("")

    lines.append("")
    f.write("\n".join(lines).encode("utf-8"))

    f.write(br'''
#endif // VK_LOADER_H''')

with open("loader.cc", "wb") as f:
    if vulkan_h_copyright:
        f.write(vulkan_h_copyright.encode("utf-8"))
    if platform_h_copyright:
        f.write(platform_h_copyright.encode("utf-8"))

    f.write(br'''#include <turf/c/core.h>
#ifdef TURF_TARGET_WIN32
#  define VK_USE_PLATFORM_WIN32_KHR
#else
#  define VK_USE_PLATFORM_XLIB_KHR
#endif
#include "loader.h"
#include <cstring>
#include <vector>
#include <unordered_map>

#if TURF_TARGET_POSIX
#  include <dlfcn.h>
#endif

namespace std {

template <>
struct is_error_code_enum<vk::loader_result> : public true_type {};

} // namespace std

std::string vk::loader_result_category_impl::message(int ev) const {
  switch (static_cast<vk::loader_result>(ev)) {
  case vk::loader_result::success: return "Success";
  case vk::loader_result::no_library: return "No library";
  case vk::loader_result::no_function: return "No function";
  }
#ifdef TURF_COMPILER_MSVC
  __assume(0);
#else
  __builtin_unreachable();
#endif
} // vk::loader_result_category_impl::message

std::error_category const& vk::loader_result_category() {
  static loader_result_category_impl instance;
  return instance;
} // vk::loader_result_category

static void* s_lib_handle{nullptr};
static std::vector<VkExtensionProperties> s_instance_extensions{};
static std::unordered_map<VkPhysicalDevice, std::vector<VkExtensionProperties>>
  s_device_extensions{};

''')

    lines = []
    lines.append("// Functions")

    for platform in sorted(platform_funcs):
        if platform:
            lines.append("#ifdef " + platform)
        for func in sorted(platform_funcs[platform]):
            lines.append("PFN_{0} _{0};".format(func))
        if platform:
            lines.append("#endif // " + platform)
        lines.append("")

    lines.append("")
    f.write("\n".join(lines).encode("utf-8"))

    f.write(br'''
static PFN_vkVoidFunction
load_proc_addr(char const* name, std::error_code& ec) noexcept {
  PFN_vkVoidFunction fp;

#if TURF_TARGET_WIN32
  fp = reinterpret_cast<PFN_vkVoidFunction>(
    ::GetProcAddress(reinterpret_cast<HMODULE>(s_lib_handle), name));

#elif TURF_TARGET_POSIX
  fp = reinterpret_cast<PFN_vkVoidFunction>(::dlsym(s_lib_handle, name));

#endif

  if (!fp) ec = vk::loader_result::no_function;
  else ec.clear();
  return fp;
} // load_proc_addr

std::error_code vk::init_loader() noexcept {
  deinit_loader();

#if TURF_TARGET_WIN32
  char const* libname = "vulkan-1.dll";
#elif TURF_TARGET_POSIX
  char const* libname = "libvulkan.so.1";
#endif

#if TURF_TARGET_WIN32
  s_lib_handle = ::LoadLibrary(libname);
  if (!s_lib_handle) {
    return loader_result::no_library;
  }

#elif PLAT_TARGET_POSIX
  s_lib_handle = ::dlopen(libname, RTLD_LAZY | RTLD_LOCAL);
  if (!s_lib_handle) {
    return loader_result::no_library;
  }

#endif

  std::error_code ec;

  _vkGetInstanceProcAddr =
    reinterpret_cast<PFN_vkGetInstanceProcAddr>(
      load_proc_addr("vkGetInstanceProcAddr", ec));
  if (ec) return ec;

  _vkEnumerateInstanceExtensionProperties =
    reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
      load_proc_addr("vkEnumerateInstanceExtensionProperties", ec));
  if (ec) return ec;

  _vkEnumerateInstanceLayerProperties =
    reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(
      load_proc_addr("vkEnumerateInstanceLayerProperties", ec));
  if (ec) return ec;

  _vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(
    load_proc_addr("vkCreateInstance", ec));
  if (ec) return ec;

  return vk::loader_result::success;
} // vk::init_loader

bool vk::instance_extension_present(char const* name) {
  if (!s_lib_handle) return false;
  if (!name) return false;

  if (s_instance_extensions.empty()) {
    uint32_t count;
    VkResult rslt = vkEnumerateInstanceExtensionProperties(nullptr, &count,
                                                           nullptr);
    if (rslt != VK_SUCCESS) return false;

    s_instance_extensions.resize(count);
    rslt = vkEnumerateInstanceExtensionProperties(nullptr, &count,
      s_instance_extensions.data());
    if (rslt != VK_SUCCESS) return false;
  }

  uint32_t count = static_cast<uint32_t>(s_instance_extensions.size());
  for (uint32_t i = 0; i < count; ++i) {
    if (strcmp(s_instance_extensions[i].extensionName, name) == 0) {
      return true;
    }
  }

  return false;
} // vk::instance_extension_present

void vk::load_instance_procs(VkInstance instance) noexcept {
  if (!s_lib_handle) return;
  if (instance == VK_NULL_HANDLE) return;

''')

    lines = []
    for platform in sorted(platform_funcs):
        if platform:
            lines.append("#ifdef " + platform)
        for func in sorted(platform_funcs[platform]):
            lines.append("  _{0} =\n\
    reinterpret_cast<PFN_{0}>(\n\
        vkGetInstanceProcAddr(instance, \"{0}\"));".format(func))
        if platform:
            lines.append("#endif // " + platform)
        lines.append("")

    f.write("\n".join(lines).encode("utf-8"))

    f.write(br'''
} // vk::load_instance_procs

bool vk::device_extension_present(VkPhysicalDevice device, char const* name) {
  if (!s_lib_handle) return false;
  if (device == VK_NULL_HANDLE) return false;
  if (!name) return false;

  if (s_device_extensions.find(device) == s_device_extensions.end()) {
    uint32_t count;
    VkResult rslt = vkEnumerateDeviceExtensionProperties(device, nullptr,
                                                         &count, nullptr);
    if (rslt != VK_SUCCESS) return false;

    s_device_extensions[device].resize(count);
    rslt = vkEnumerateDeviceExtensionProperties(device, nullptr, &count,
      s_device_extensions[device].data());
    if (rslt != VK_SUCCESS) return false;
  }

  uint32_t count = static_cast<uint32_t>(s_device_extensions[device].size());
  for (uint32_t i = 0; i < count; ++i) {
    if (strcmp(s_device_extensions[device][i].extensionName, name) == 0) {
      return true;
    }
  }

  return false;
} // vk::device_extension_present

void vk::load_device_procs(VkDevice device) noexcept {
  if (!s_lib_handle) return;
  if (device == VK_NULL_HANDLE) return;

''')

    lines = []
    for platform in sorted(platform_funcs):
        if platform:
            lines.append("#ifdef " + platform)
        for func in sorted(platform_funcs[platform]):
            lines.append("  if (!_{0}) {{\n\
    _{0} =\n\
      reinterpret_cast<PFN_{0}>(\n\
        vkGetDeviceProcAddr(device, \"{0}\"));\n\
  }}".format(func))
        if platform:
            lines.append("#endif // " + platform)
        lines.append("")

    f.write("\n".join(lines).encode("utf-8"))

    f.write(br'''
} // vk::load_device_procs

void vk::deinit_loader() noexcept {
  if (!s_lib_handle) return;
        
#if TURF_TARGET_WIN32
  ::FreeLibrary(reinterpret_cast<HMODULE>(s_lib_handle));

#elif TURF_TARGET_POSIX
  ::dlclose(s_lib_handle);

#endif
    
  s_lib_handle = nullptr;
} // vk::deinit_loader

''')
