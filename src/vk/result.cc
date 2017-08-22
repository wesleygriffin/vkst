#include "result.h"

namespace std {

template <>
struct is_error_code_enum<vk::result> : public true_type {};

template <>
struct is_error_code_enum<vk::shaderc_result> : public true_type {};

} // namespace std

std::string vk::result_category_impl::message(int ev) const {
  switch (static_cast<vk::result>(ev)) {
  case vk::result::success: return "Success";
  case vk::result::not_ready: return "Not ready";
  case vk::result::timeout: return "Timeout";
  case vk::result::event_set: return "Event set";
  case vk::result::event_reset: return "Event reset";
  case vk::result::incomplete: return "Incomplete";
  case vk::result::error_out_of_host_memory: return "Out of host memory";
  case vk::result::error_out_of_device_memory: return "Out of device memory";
  case vk::result::error_initialization_failed: return "Initialization failed";
  case vk::result::error_device_lost: return "Device lost";
  case vk::result::error_memory_map_failed: return "Memory map failed";
  case vk::result::error_layer_not_present: return "Layer not present";
  case vk::result::error_extension_not_present: return "Extension not present";
  case vk::result::error_feature_not_present: return "Feature not present";
  case vk::result::error_incompatible_driver: return "Incompatible driver";
  case vk::result::error_too_many_objects: return "Too many objects";
  case vk::result::error_format_not_supported: return "Format not supported";
  case vk::result::error_fragmented_pool: return "Fragmented pool";
  case vk::result::error_surface_lost_khr: return "Surface lost";
  case vk::result::error_native_window_in_use_khr:
    return "Native window in use";
  case vk::result::suboptimal_khr: return "Suboptimal";
  case vk::result::error_out_of_date_khr: return "Out of date";
  case vk::result::error_incompatible_display_khr:
    return "Incompatible display";
  case vk::result::error_validation_failed_ext: return "Validation failed";
  case vk::result::error_invalid_shader_nv: return "Invalid shader";
  case vk::result::error_out_of_pool_memory_khr: return "Out of pool memory";
  case vk::result::error_invalid_external_handle_khx:
    return "Invalid external handle";
  }
} // vk::result_category_impl::message

std::error_category const& vk::result_category() {
  static result_category_impl instance;
  return instance;
} // vk::result_category

std::string vk::shaderc_result_category_impl::message(int ev) const {
  switch (static_cast<vk::shaderc_result>(ev)) {
  case vk::shaderc_result::success: return "Success";
  case vk::shaderc_result::invalid_stage: return "Invalid stage";
  case vk::shaderc_result::compilation_error: return "Compilation error";
  case vk::shaderc_result::internal_error: return "Internal error";
  case vk::shaderc_result::null_result_object: return "Null result object";
  case vk::shaderc_result::invalid_assembly: return "Invalid assembly";
  }
} // vk::shaderc_result_category_impl::message

std::error_category const& vk::shaderc_result_category() {
  static shaderc_result_category_impl instance;
  return instance;
} // vk::shaderc_result_category
