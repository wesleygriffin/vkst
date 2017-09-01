#ifndef VKST_VK_RESULT_H
#define VKST_VK_RESULT_H

#include <turf/c/core.h>

#ifdef TURF_TARGET_WIN32
#  define VK_USE_PLATFORM_WIN32_KHR
#else
#  define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>

#include <shaderc/shaderc.hpp>

#include <system_error>

namespace vk {

enum class result {
  success = 0,
  not_ready = 1,
  timeout = 2,
  event_set = 3,
  event_reset = 4,
  incomplete = 5,

  error_out_of_host_memory = -1,
  error_out_of_device_memory = -2,
  error_initialization_failed = -3,
  error_device_lost = -4,
  error_memory_map_failed = -5,
  error_layer_not_present = -6,
  error_extension_not_present = -7,
  error_feature_not_present = -8,
  error_incompatible_driver = -9,
  error_too_many_objects = -10,
  error_format_not_supported = -11,
  error_fragmented_pool = -12,

  error_surface_lost_khr = -1000000000,
  error_native_window_in_use_khr = -1000000001,
  suboptimal_khr = 1000001003,
  error_out_of_date_khr = -1000001004,
  error_incompatible_display_khr = -1000003001,
  error_validation_failed_ext = -1000011001,
  error_invalid_shader_nv = -1000012000,
  error_out_of_pool_memory_khr = -1000069000,
  error_invalid_external_handle_khx = -1000072003,
}; // enum class result

class result_category_impl : public std::error_category {
public:
  virtual char const* name() const noexcept override { return "vk::result"; }

  virtual std::string message(int ev) const override;
}; // class result_category_impl

std::error_category const& result_category();

inline std::error_code make_error_code(result e) noexcept {
  return {static_cast<int>(e), result_category()};
}

inline std::error_code make_error_code(VkResult r) noexcept {
  return {static_cast<int>(r), result_category()};
}

enum class shaderc_result {
  success = 0,
  invalid_stage,
  compilation_error,
  internal_error,
  null_result_object,
  invalid_assembly,
}; // enum class shaderc_result

class shaderc_result_category_impl : public std::error_category {
public:
  virtual char const* name() const noexcept override {
    return "vk::shaderc_result";
  }

  virtual std::string message(int ev) const override;
}; // class shaderc_result_category_impl

std::error_category const& shaderc_result_category();

inline std::error_code make_error_code(shaderc_result e) noexcept {
  return {static_cast<int>(e), shaderc_result_category()};
}

inline std::error_code make_error_code(shaderc_compilation_status s) noexcept {
  return {static_cast<int>(s), shaderc_result_category()};
}

} // namespace vk

#endif // VKST_VK_RESULT_H