#include "renderer.h"
#include <plat/core.h>
#include <plat/file_io.h>
#include <plat/log.h>
#include <shaderc/shaderc.hpp>

surface::surface(surface&& other) noexcept
: _surface{other._surface}
, _color_format{other._color_format}
, _depth_format{other._depth_format}
, _samples{other._samples}
, _present_mode{other._present_mode}
, _image_available{other._image_available}
, _render_finished{other._render_finished}
, _render_pass{other._render_pass}
, _extent{other._extent}
, _viewport{other._viewport}
, _scissor{other._scissor}
, _capabilities{other._capabilities}
, _swapchain{other._swapchain}
, _color_images{std::move(other._color_images)}
, _color_image_views{std::move(other._color_image_views)}
, _depth_image{other._depth_image}
, _depth_image_memory{other._depth_image_memory}
, _depth_image_view{other._depth_image_view}
, _color_target{other._color_target}
, _color_target_memory{other._color_target_memory}
, _color_target_view{other._color_target_view}
, _depth_target{other._depth_target}
, _depth_target_memory{other._depth_target_memory}
, _depth_target_view{other._depth_target_view}
, _framebuffers{std::move(other._framebuffers)} {

  other._surface = VK_NULL_HANDLE;
  other._image_available = other._render_finished = VK_NULL_HANDLE;
  other._render_pass = VK_NULL_HANDLE;
  other._swapchain = VK_NULL_HANDLE;

  other._depth_image = other._color_target = other._depth_target =
    VK_NULL_HANDLE;
  other._depth_image_memory = other._color_target_memory =
    other._depth_target_memory = VK_NULL_HANDLE;
  other._depth_image_view = other._color_target_view =
    other._depth_target_view = VK_NULL_HANDLE;
} // surface::surface

surface& surface::operator=(surface&& rhs) noexcept {
  if (this == &rhs) return *this;

  _surface = rhs._surface;
  _color_format = rhs._color_format;
  _depth_format = rhs._depth_format;
  _samples = rhs._samples;
  _present_mode = rhs._present_mode;
  _image_available = rhs._image_available;
  _render_finished = rhs._render_finished;
  _render_pass = rhs._render_pass;
  _extent = rhs._extent;
  _viewport = rhs._viewport;
  _scissor = rhs._scissor;
  _capabilities = rhs._capabilities;
  _swapchain = rhs._swapchain;
  _color_images = std::move(rhs._color_images);
  _color_image_views = std::move(rhs._color_image_views);
  _depth_image = rhs._depth_image;
  _depth_image_memory = rhs._depth_image_memory;
  _depth_image_view = rhs._depth_image_view;
  _color_target = rhs._color_target;
  _color_target_memory = rhs._color_target_memory;
  _color_target_view = rhs._color_target_view;
  _depth_target = rhs._depth_target;
  _depth_target_memory = rhs._depth_target_memory;
  _depth_target_view = rhs._depth_target_view;
  _framebuffers = std::move(rhs._framebuffers);

  rhs._surface = VK_NULL_HANDLE;
  rhs._image_available = rhs._render_finished = VK_NULL_HANDLE;
  rhs._render_pass = VK_NULL_HANDLE;
  rhs._swapchain = VK_NULL_HANDLE;

  rhs._depth_image = rhs._color_target = rhs._depth_target = VK_NULL_HANDLE;
  rhs._depth_image_memory = rhs._color_target_memory =
    rhs._depth_target_memory = VK_NULL_HANDLE;
  rhs._depth_image_view = rhs._color_target_view = rhs._depth_target_view =
    VK_NULL_HANDLE;

  return *this;
} // surface::operator=

shader::shader(shader&& other) noexcept : _module{other._module} {
  other._module = VK_NULL_HANDLE;
}

shader& shader::operator=(shader&& rhs) noexcept {
  if (this == &rhs) return *this;
  _module = rhs._module;
  rhs._module = VK_NULL_HANDLE;
  return *this;
}

namespace std {

template <>
struct is_error_code_enum<renderer_result> : public true_type {};

} // namespace std

std::string renderer_result_category_impl::message(int ev) const {
  switch (static_cast<renderer_result>(ev)) {
  case renderer_result::success: return "Success";
  case renderer_result::no_device: return "No device";
  case renderer_result::initialization_failed: return "Initialization failed";
  case renderer_result::surface_not_supported: return "Surface not supported";
  case renderer_result::no_memory_type: return "No memory type";
  }
  PLAT_MARK_UNREACHABLE;
} // renderer_result_category_impl::message

std::error_category const& renderer_result_category() {
  static renderer_result_category_impl instance;
  return instance;
} // renderer_result_category

static VkInstance create_instance(gsl::czstring application_name,
                                  std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  std::array<gsl::czstring, 1> layers{{"VK_LAYER_LUNARG_standard_validation"}};
  // I don't like GOOGLE_unique_objects, as it obscures the true vulkan handles
  // But it is included in VK_LAYER_LUNARG_standard_validation. My preference is:

  //std::array<gsl::czstring, 4> layers{
  //  {"VK_LAYER_LUNARG_core_validation", "VK_LAYER_LUNARG_object_tracker",
  //   "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_GOOGLE_threading"}};


  std::array<gsl::czstring, 3> extensions{{
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    VK_KHR_SURFACE_EXTENSION_NAME,
#if TURF_TARGET_WIN32
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
  }};

  VkApplicationInfo ainfo = {};
  ainfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  ainfo.pApplicationName = application_name;
  ainfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo cinfo = {};
  cinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  cinfo.pApplicationInfo = &ainfo;
  cinfo.enabledLayerCount = gsl::narrow_cast<uint32_t>(layers.size());
  cinfo.ppEnabledLayerNames = layers.data();
  cinfo.enabledExtensionCount = gsl::narrow_cast<uint32_t>(extensions.size());
  cinfo.ppEnabledExtensionNames = extensions.data();

  VkInstance instance;
  VkResult rslt = vkCreateInstance(&cinfo, nullptr, &instance);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_NULL_HANDLE;
  }

  vk::load_instance_procs(instance);

  LOG_LEAVE;
  return instance;
} // create_instance

static VkDebugReportCallbackEXT
create_debug_report_callback(VkInstance instance,
                             PFN_vkDebugReportCallbackEXT callback,
                             std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  VkDebugReportCallbackCreateInfoEXT drccinfo = {};
  drccinfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  drccinfo.flags = VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;
  drccinfo.pfnCallback = callback;

  VkDebugReportCallbackEXT debug_report_callback;
  VkResult rslt = vkCreateDebugReportCallbackEXT(instance, &drccinfo, nullptr,
                                                 &debug_report_callback);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_NULL_HANDLE;
  }

  LOG_LEAVE;
  return debug_report_callback;
} // create_debug_report_callback

static std::pair<VkPhysicalDevice, uint32_t>
find_physical(VkInstance instance, uint32_t push_constant_size,
              std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  if (push_constant_size > 128) {
    LOG_WARN("requested size of push constants (%d) is > 128",
             push_constant_size);
  }

  std::array<VkPhysicalDevice, 4> devices;
  uint32_t num_devices = gsl::narrow_cast<uint32_t>(devices.max_size());
  VkResult rslt =
    vkEnumeratePhysicalDevices(instance, &num_devices, devices.data());
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return {VK_NULL_HANDLE, UINT32_MAX};
  }

  VkPhysicalDeviceProperties properties;
  std::vector<VkQueueFamilyProperties> families;

  for (uint32_t i = 0; i < num_devices; ++i) {
    auto&& device = devices[i];
    
    vkGetPhysicalDeviceProperties(device, &properties);
    if (properties.limits.maxPushConstantsSize < push_constant_size) continue;

    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

    families.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

    for (uint32_t j = 0; j < count; ++j) {
      if (families[j].queueCount == 0) continue;
      if ((families[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) !=
          VK_QUEUE_GRAPHICS_BIT)
        continue;

#if TURF_TARGET_WIN32
      if (!vkGetPhysicalDeviceWin32PresentationSupportKHR(device, j)) continue;
#elif TURF_KERNEL_LINUX
      if (!vkGetPhysicalDeviceXlibPresentationSupportKHR(device, j)) continue;
#endif

      LOG_LEAVE;
      return {device, j};
    }
  }

  ec.assign(static_cast<int>(renderer_result::no_device),
            renderer_result_category());
  return {VK_NULL_HANDLE, UINT32_MAX};
} // find_physical

static std::pair<VkDevice, VkQueue>
create_device(VkPhysicalDevice physical, uint32_t queue_family,
              std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  float const priority = 1.f;
  VkDeviceQueueCreateInfo qcinfo = {};
  qcinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  qcinfo.queueFamilyIndex = queue_family;
  qcinfo.queueCount = 1;
  qcinfo.pQueuePriorities = &priority;

  std::array<gsl::czstring, 1> extensions{{VK_KHR_SWAPCHAIN_EXTENSION_NAME}};

  for (auto&& extension : extensions) {
    if (!vk::device_extension_present(physical, extension)) {
      ec.assign(static_cast<int>(vk::result::error_extension_not_present),
                vk::result_category());
      return {};
    }
  }

  VkPhysicalDeviceFeatures features;

  // clang-format off
  features.robustBufferAccess = VK_FALSE;
  features.fullDrawIndexUint32 = VK_TRUE;
  features.imageCubeArray = VK_TRUE;
  features.independentBlend = VK_FALSE; // VK_TRUE?
  features.geometryShader = VK_TRUE;
  features.tessellationShader = VK_TRUE;
  features.sampleRateShading = VK_FALSE; // VK_TRUE?
  features.dualSrcBlend = VK_FALSE; // VK_TRUE?
  features.logicOp = VK_FALSE; // VK_TRUE?
  features.multiDrawIndirect = VK_FALSE; // VK_TRUE?
  features.drawIndirectFirstInstance = VK_FALSE; // VK_TRUE?
  features.depthClamp = VK_TRUE;
  features.depthBiasClamp = VK_TRUE;
  features.fillModeNonSolid = VK_TRUE;
  features.depthBounds = VK_FALSE; // VK_TRUE?
  features.wideLines = VK_FALSE; // VK_TRUE?
  features.largePoints = VK_FALSE; // VK_TRUE?
  features.alphaToOne = VK_FALSE; // VK_TRUE?
  features.multiViewport = VK_TRUE;
  features.samplerAnisotropy = VK_TRUE;
  features.textureCompressionETC2 = VK_FALSE;
  features.textureCompressionASTC_LDR = VK_FALSE; // BOO!!
  features.textureCompressionBC = VK_TRUE;
  features.occlusionQueryPrecise = VK_FALSE; // VK_TRUE?
  features.pipelineStatisticsQuery = VK_TRUE;
  features.vertexPipelineStoresAndAtomics = VK_FALSE; // VK_TRUE?
  features.fragmentStoresAndAtomics = VK_FALSE; // VK_TRUE?
  features.shaderTessellationAndGeometryPointSize = VK_FALSE; // VK_TRUE?
  features.shaderImageGatherExtended = VK_FALSE; // VK_TRUE?
  features.shaderStorageImageExtendedFormats = VK_FALSE; // VK_TRUE?
  features.shaderStorageImageMultisample = VK_FALSE; // VK_TRUE?
  features.shaderStorageImageReadWithoutFormat = VK_FALSE; // VK_TRUE?
  features.shaderStorageImageWriteWithoutFormat = VK_FALSE; // VK_TRUE?
  features.shaderUniformBufferArrayDynamicIndexing = VK_FALSE; // VK_TRUE?
  features.shaderSampledImageArrayDynamicIndexing = VK_FALSE; // VK_TRUE?
  features.shaderStorageBufferArrayDynamicIndexing = VK_FALSE; // VK_TRUE?
  features.shaderStorageImageArrayDynamicIndexing = VK_FALSE; // VK_TRUE?
  features.shaderClipDistance = VK_TRUE;
  features.shaderCullDistance = VK_TRUE;
  features.shaderFloat64 = VK_FALSE;
  features.shaderInt64 = VK_FALSE;
  features.shaderInt16 = VK_FALSE;
  features.shaderResourceResidency = VK_FALSE; // VK_TRUE?
  features.shaderResourceMinLod = VK_TRUE;
  features.sparseBinding = VK_FALSE; // VK_TRUE?
  features.sparseResidencyBuffer = VK_FALSE; // VK_TRUE?
  features.sparseResidencyImage2D = VK_FALSE; // VK_TRUE?
  features.sparseResidencyImage3D = VK_FALSE; // VK_TRUE?
  features.sparseResidency2Samples = VK_FALSE; // VK_TRUE?
  features.sparseResidency4Samples = VK_FALSE; // VK_TRUE?
  features.sparseResidency8Samples = VK_FALSE; // VK_TRUE?
  features.sparseResidency16Samples = VK_FALSE; // VK_TRUE?
  features.sparseResidencyAliased = VK_FALSE; // VK_TRUE?
  features.variableMultisampleRate = VK_FALSE; // VK_TRUE?
  features.inheritedQueries = VK_FALSE; // VK_TRUE?
  // clang-format on

  VkDeviceCreateInfo cinfo = {};
  cinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  cinfo.queueCreateInfoCount = 1;
  cinfo.pQueueCreateInfos = &qcinfo;
  cinfo.enabledExtensionCount = gsl::narrow_cast<uint32_t>(extensions.size());
  cinfo.ppEnabledExtensionNames = extensions.data();
  cinfo.pEnabledFeatures = &features;

  VkDevice device;
  VkResult rslt = vkCreateDevice(physical, &cinfo, nullptr, &device);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return {};
  }

  vk::load_device_procs(device);

  VkQueue queue;
  vkGetDeviceQueue(device, queue_family, 0, &queue);

  LOG_LEAVE;
  return std::make_pair(device, queue);
} // create_device

static VkCommandPool create_command_pool(VkDevice device, uint32_t queue_family,
                                         std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  VkCommandPoolCreateInfo cpcinfo = {};
  cpcinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cpcinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  cpcinfo.queueFamilyIndex = queue_family;

  VkCommandPool command_pool;
  VkResult rslt = vkCreateCommandPool(device, &cpcinfo, nullptr, &command_pool);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_NULL_HANDLE;
  }

  LOG_LEAVE;
  return command_pool;
} // create_command_pool

static VkFence create_fence(VkDevice device, std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  VkFenceCreateInfo fcinfo = {};
  fcinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

  VkFence fence;
  VkResult rslt = vkCreateFence(device, &fcinfo, nullptr, &fence);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_NULL_HANDLE;
  }

  LOG_LEAVE;
  return fence;
} // create_device

renderer renderer::create(gsl::czstring application_name,
                          PFN_vkDebugReportCallbackEXT callback,
                          uint32_t push_constant_size,
                          std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  //
  // This function has been subdivided into smaller functions for readability.
  // Those functions are all defined above and commented on there.
  //

  renderer r;

  // Start by initializing the loader. This loads the Vulkan shared library
  // and initializes the needed non-instance-based function pointers.
  ec = vk::init_loader();
  if (ec) return r;

  r._instance = ::create_instance(application_name, ec);
  if (ec) return r;

  r._callback = ::create_debug_report_callback(r._instance, callback, ec);
  if (ec) return r;

  std::tie(r._physical, r._graphics_queue_family_index) =
    ::find_physical(r._instance, push_constant_size, ec);
  if (ec) return r;

  std::tie(r._device, r._graphics_queue) =
    ::create_device(r._physical, r._graphics_queue_family_index, ec);
  if (ec) return r;

  r._graphics_command_pool =
    ::create_command_pool(r._device, r._graphics_queue_family_index, ec);
  if (ec) return r;

  r._graphics_onetime_fence = ::create_fence(r._device, ec);
  if (ec) return r;

  LOG_LEAVE;
  return r;
} // renderer::create

static VkSurfaceKHR create_surface(VkInstance instance,
                                   wsi::window const& window,
                                   std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  auto&& native = window.native_handle();
  VkSurfaceKHR surface;

#if TURF_TARGET_WIN32
  VkWin32SurfaceCreateInfoKHR cinfo = {};
  cinfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  std::tie(cinfo.hinstance, cinfo.hwnd) = native;

  VkResult rslt = vkCreateWin32SurfaceKHR(instance, &cinfo, nullptr, &surface);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_NULL_HANDLE;
  }

#elif TURF_KERNEL_LINUX
  VkXlibSurfaceCreateInfoKHR cinfo = {};
  cinfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
  std::tie(cinfo.dpy, std::ignore, cinfo.window) + native;

  VkSurfaceKHR surface;
  VkResult rslt = vkCreateXlibSurfaceKHR(instance, &cinfo, nullptr, &surface);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_NULL_HANDLE;
  }

#endif

  LOG_LEAVE;
  return surface;
} // create_surface

static bool check_surface_support(VkPhysicalDevice physical,
                                  uint32_t queue_family, VkSurfaceKHR surface,
                                  std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  VkBool32 supports;
  VkResult rslt = vkGetPhysicalDeviceSurfaceSupportKHR(physical, queue_family,
                                                       surface, &supports);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return false;
  };

  LOG_LEAVE;
  return (supports == VK_TRUE);
} // check_surface_support

static VkSurfaceFormatKHR choose_surface_format(VkPhysicalDevice physical,
                                                VkSurfaceKHR surface,
                                                VkSurfaceFormatKHR desired,
                                                std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  uint32_t count;
  VkResult rslt =
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &count, nullptr);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return {VK_FORMAT_UNDEFINED, VK_COLORSPACE_SRGB_NONLINEAR_KHR};
  }

  std::vector<VkSurfaceFormatKHR> formats(count);
  rslt = vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &count,
                                              formats.data());
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return {VK_FORMAT_UNDEFINED, VK_COLORSPACE_SRGB_NONLINEAR_KHR};
  }


  if (count == 1 && formats[0].format == VK_FORMAT_UNDEFINED) return desired;

  for (auto&& format : formats) {
    if (format.format == desired.format &&
        format.colorSpace == desired.colorSpace) {
      LOG_LEAVE;
      return desired;
    }
  }

  LOG_LEAVE;
  return formats[0];
} // choose_surface_format

static VkPresentModeKHR choose_present_mode(VkPhysicalDevice physical,
                                            VkSurfaceKHR surface,
                                            VkPresentModeKHR desired,
                                            std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  uint32_t count;
  VkResult rslt = vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface,
                                                            &count, nullptr);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_PRESENT_MODE_FIFO_KHR;
  }

  std::vector<VkPresentModeKHR> modes(count);
  rslt = vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface, &count,
                                                   modes.data());
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_PRESENT_MODE_FIFO_KHR;
  }

  for (auto&& mode : modes) {
    if (mode == desired) {
      LOG_LEAVE;
      return desired;
    }
  }

  LOG_LEAVE;
  return VK_PRESENT_MODE_FIFO_KHR; // required by spec to be supported
} // choose_present_mode

static VkSemaphore create_semaphore(VkDevice device,
                                    std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  VkSemaphoreCreateInfo cinfo = {};
  cinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkSemaphore semaphore;
  VkResult rslt = vkCreateSemaphore(device, &cinfo, nullptr, &semaphore);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_NULL_HANDLE;
  }

  LOG_LEAVE;
  return semaphore;
} // create_semaphore

static VkRenderPass create_render_pass(VkFormat color_format,
                                       VkFormat depth_format, VkDevice device,
                                       std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  std::array<VkAttachmentDescription, 4> attachments{
    {{0, color_format, VK_SAMPLE_COUNT_8_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
      VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
     {0, color_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
     {0, depth_format, VK_SAMPLE_COUNT_8_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
      VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
     {0, depth_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}}};

  VkAttachmentReference color{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  VkAttachmentReference resolve{1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  VkAttachmentReference depth_stencil{
    2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

  VkSubpassDescription subpasses = {0,        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    0,        nullptr,
                                    1,        &color,
                                    &resolve, &depth_stencil,
                                    0,        nullptr};

  std::array<VkSubpassDependency, 2> dependencies{
    {{VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_MEMORY_READ_BIT,
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      VK_DEPENDENCY_BY_REGION_BIT},
     {0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT}}};

  VkRenderPassCreateInfo cinfo = {};
  cinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  cinfo.attachmentCount = gsl::narrow_cast<uint32_t>(attachments.size());
  cinfo.pAttachments = attachments.data();
  cinfo.subpassCount = 1;
  cinfo.pSubpasses = &subpasses;
  cinfo.dependencyCount = gsl::narrow_cast<uint32_t>(dependencies.size());
  cinfo.pDependencies = dependencies.data();

  VkRenderPass render_pass;
  VkResult rslt = vkCreateRenderPass(device, &cinfo, nullptr, &render_pass);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_NULL_HANDLE;
  }

  LOG_LEAVE;
  return render_pass;
} // create_render_pass

surface renderer::create_surface(wsi::window const& window,
                                 std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  //
  // This function has been subdivided into smaller functions for readability.
  // Those functions are all defined above and commented on there.
  //

  surface s;
  s._surface = ::create_surface(_instance, window, ec);
  if (ec) return s;

  bool supports = ::check_surface_support(
    _physical, _graphics_queue_family_index, s._surface, ec);
  if (ec) return s;
  if (!supports) {
    ec.assign(static_cast<int>(renderer_result::surface_not_supported),
              renderer_result_category());
    return s;
  }

  // The desired surface format passed in to choose_surface_format
  s._color_format = {VK_FORMAT_B8G8R8A8_UNORM,
                     VK_COLORSPACE_SRGB_NONLINEAR_KHR};
  s._color_format =
    ::choose_surface_format(_physical, s._surface, s._color_format, ec);
  if (ec) return s;

  // Hard-coded for convenience
  s._depth_format = VK_FORMAT_D32_SFLOAT;
  s._samples = VK_SAMPLE_COUNT_8_BIT;

  // The desired present mode passed in to choose_present_mode
  s._present_mode = VK_PRESENT_MODE_FIFO_KHR;
  s._present_mode =
    ::choose_present_mode(_physical, s._surface, s._present_mode, ec);
  if (ec) return s;

  s._image_available = ::create_semaphore(_device, ec);
  if (ec) return s;

  s._render_finished = ::create_semaphore(_device, ec);
  if (ec) return s;

  s._render_pass =
    ::create_render_pass(s._color_format.format, s._depth_format, _device, ec);
  if (ec) return s;

  resize(s, window.size(), ec);
  if (ec) return s;

  LOG_LEAVE;
  return s;
} // renderer::create_surface

static VkSurfaceCapabilitiesKHR
get_surface_capabilities(VkPhysicalDevice physical, VkSurfaceKHR surface,
                         std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  VkSurfaceCapabilitiesKHR capabilities;
  VkResult rslt =
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, surface, &capabilities);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return capabilities;
  }

  LOG_LEAVE;
  return capabilities;
} // get_surface_capabilities

static VkSwapchainKHR
create_swapchain(VkDevice device, VkSurfaceKHR surface,
                 VkSurfaceCapabilitiesKHR capabilities, VkExtent2D extent,
                 VkSurfaceFormatKHR color_format, VkPresentModeKHR present_mode,
                 VkSwapchainKHR old_swapchain, std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  VkSwapchainCreateInfoKHR cinfo;
  cinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  cinfo.pNext = nullptr;
  cinfo.flags = 0;
  cinfo.surface = surface;
  cinfo.minImageCount = capabilities.minImageCount;
  cinfo.imageFormat = color_format.format;
  cinfo.imageColorSpace = color_format.colorSpace;
  cinfo.imageExtent = extent;
  cinfo.imageArrayLayers = 1;
  cinfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  cinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  cinfo.queueFamilyIndexCount = 0;
  cinfo.pQueueFamilyIndices = nullptr;
  cinfo.preTransform = capabilities.currentTransform;
  cinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  cinfo.presentMode = present_mode;
  cinfo.clipped = true;
  cinfo.oldSwapchain = old_swapchain;

  VkSwapchainKHR swapchain;
  VkResult rslt = vkCreateSwapchainKHR(device, &cinfo, nullptr, &swapchain);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_NULL_HANDLE;
  }

  LOG_LEAVE;
  return swapchain;
} // create_swapchain

static VkImageView create_image_view(VkDevice device, VkImage image,
                                     VkImageViewType type, VkFormat format,
                                     VkImageSubresourceRange isr,
                                     std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  VkImageViewCreateInfo cinfo = {};
  cinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  cinfo.image = image;
  cinfo.viewType = type;
  cinfo.format = format;
  cinfo.components = {
    VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
    VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
  cinfo.subresourceRange = isr;

  VkImageView view;
  VkResult rslt = vkCreateImageView(device, &cinfo, nullptr, &view);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_NULL_HANDLE;
  }

  LOG_LEAVE;
  return view;
} // create_image_view

std::pair<std::vector<VkImage>, std::vector<VkImageView>>
get_swapchain_images(VkDevice device, VkSwapchainKHR swapchain, VkFormat format,
                     std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  uint32_t count;
  VkResult rslt = vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return {};
  }

  std::vector<VkImage> images(count);
  rslt = vkGetSwapchainImagesKHR(device, swapchain, &count, images.data());
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return {};
  }

  std::vector<VkImageView> views;
  views.reserve(count);
  for (auto&& image : images) {
    views.push_back(
      ::create_image_view(device, image, VK_IMAGE_VIEW_TYPE_2D, format,
                          {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}, ec));
    if (ec) return {};
  }

  LOG_LEAVE;
  return std::make_pair(images, views);
} // get_swapchain_images

static VkImage create_image(VkDevice device, VkImageType type, VkFormat format,
                            VkExtent3D extent, VkImageUsageFlags usage,
                            uint32_t mip_levels, uint32_t array_layers,
                            VkImageLayout initial,
                            VkSampleCountFlagBits samples,
                            VkImageCreateFlags flags,
                            std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  VkImageCreateInfo cinfo = {};
  cinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  cinfo.flags = flags;
  cinfo.imageType = type;
  cinfo.format = format;
  cinfo.extent = extent;
  cinfo.mipLevels = mip_levels;
  cinfo.arrayLayers = array_layers;
  cinfo.samples = samples;
  cinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  cinfo.usage = usage;
  cinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  cinfo.initialLayout = initial;

  VkImage image;
  VkResult rslt = vkCreateImage(device, &cinfo, nullptr, &image);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_NULL_HANDLE;
  }

  LOG_LEAVE;
  return image;
} // create_image

static int32_t
find_memory_type(VkPhysicalDeviceMemoryProperties memory_properties,
                 uint32_t memory_type_bits,
                 VkMemoryPropertyFlags properties) noexcept {
  for (int32_t i = 0;
       i < static_cast<int32_t>(memory_properties.memoryTypeCount); ++i) {
    auto const& memory_type = memory_properties.memoryTypes[i];
    if ((memory_type_bits & (1 << i)) &&
        ((memory_type.propertyFlags & properties) == properties)) {
      return i;
    }
  }
  return -1;
} // find_memory_type

static VkDeviceMemory allocate_memory(VkPhysicalDevice physical,
                                      VkDevice device,
                                      VkMemoryRequirements requirements,
                                      VkMemoryPropertyFlags flags,
                                      std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(physical, &memory_properties);

  int32_t type_index = find_memory_type(memory_properties,
                                        requirements.memoryTypeBits, flags);
  if (type_index < 0) {
    ec.assign(static_cast<int>(renderer_result::no_memory_type),
              renderer_result_category());
    return VK_NULL_HANDLE;
  }

  VkMemoryAllocateInfo ainfo = {};
  ainfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  ainfo.allocationSize = requirements.size;
  ainfo.memoryTypeIndex = type_index;

  VkDeviceMemory memory;
  VkResult rslt = vkAllocateMemory(device, &ainfo, nullptr, &memory);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_NULL_HANDLE;
  }

  LOG_LEAVE;
  return memory;
} // allocate_memory

inline VkDeviceMemory allocate_memory(VkPhysicalDevice physical,
                                      VkDevice device, VkImage image,
                                      VkMemoryPropertyFlags flags,
                                      std::error_code& ec) noexcept {
  VkMemoryRequirements requirements;
  vkGetImageMemoryRequirements(device, image, &requirements);
  return allocate_memory(physical, device, requirements, flags, ec);
} // allocate_memory

inline VkDeviceMemory allocate_memory(VkPhysicalDevice physical,
                                      VkDevice device, VkBuffer buffer,
                                      VkMemoryPropertyFlags flags,
                                      std::error_code& ec) noexcept {
  VkMemoryRequirements requirements;
  vkGetBufferMemoryRequirements(device, buffer, &requirements);
  return allocate_memory(physical, device, requirements, flags, ec);
} // allocate_memory

static std::tuple<VkImage, VkDeviceMemory, VkImageView> create_image_and_view(
  VkPhysicalDevice physical, VkDevice device, VkImageType type, VkFormat format,
  VkExtent3D extent, VkImageUsageFlags usage, uint32_t mip_levels,
  uint32_t array_layers, VkImageLayout initial, VkSampleCountFlagBits samples,
  VkImageCreateFlags flags, VkImageViewType view_type,
  VkImageSubresourceRange isr, std::error_code& ec) noexcept {
  VkImage image = create_image(device, type, format, extent, usage, mip_levels,
                               array_layers, initial, samples, flags, ec);
  if (ec) return {};

  VkDeviceMemory memory = allocate_memory(
    physical, device, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ec);
  if (ec) return {};

  VkResult rslt = vkBindImageMemory(device, image, memory, 0);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return {};
  }

  VkImageView view =
    create_image_view(device, image, view_type, format, isr, ec);
  if (ec) return {};

  LOG_LEAVE;
  return std::make_tuple(image, memory, view);
} // create_image_and_view

static void transition_depth_image(renderer* r, VkImage image,
                                   std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  VkCommandBuffer command_buffer = r->allocate_command_buffers(1, ec)[0];
  if (ec) return;

  VkCommandBufferBeginInfo binfo = {};
  binfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  binfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

  vkBeginCommandBuffer(command_buffer, &binfo);

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};

  vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);

  vkEndCommandBuffer(command_buffer);

  r->submit(command_buffer, true, ec);
} // transition_depth_image

static std::vector<VkFramebuffer>
create_framebuffers(VkDevice device, gsl::span<VkImageView> attachments,
                    gsl::span<VkImageView> image_views,
                    VkRenderPass render_pass, VkExtent2D extent,
                    std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  std::vector<VkFramebuffer> framebuffers;
  framebuffers.reserve(image_views.size());

  VkFramebufferCreateInfo cinfo = {};
  cinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  cinfo.renderPass = render_pass;
  cinfo.attachmentCount = 4;
  cinfo.width = extent.width;
  cinfo.height = extent.height;
  cinfo.layers = 1;

  for (auto&& view : image_views) {
    attachments[1] = view;
    cinfo.pAttachments = attachments.data();

    VkFramebuffer framebuffer;
    VkResult rslt = vkCreateFramebuffer(device, &cinfo, nullptr, &framebuffer);
    if (rslt != VK_SUCCESS) {
      ec.assign(rslt, vk::result_category());
      return {};
    }

    framebuffers.push_back(framebuffer);
  }

  LOG_LEAVE;
  return framebuffers;
} // create_framebuffers

void renderer::resize(surface& s, wsi::extent2d const& extent,
                      std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  VkSurfaceCapabilitiesKHR new_capabilities =
    ::get_surface_capabilities(_physical, s._surface, ec);
  if (ec) return;

  VkExtent2D new_extent = {
    new_capabilities.currentExtent.width == UINT32_MAX
      ? std::max(std::min(static_cast<uint32_t>(extent.width),
                          new_capabilities.maxImageExtent.width),
                 new_capabilities.minImageExtent.width)
      : new_capabilities.currentExtent.width,
    new_capabilities.currentExtent.height == UINT32_MAX
      ? std::max(std::min(static_cast<uint32_t>(extent.height),
                          new_capabilities.maxImageExtent.height),
                 new_capabilities.minImageExtent.height)
      : new_capabilities.currentExtent.height};

  VkExtent3D image_extent{new_extent.width, new_extent.height, 1};

  VkViewport new_viewport{0.f,
                          0.f,
                          static_cast<float>(new_extent.width),
                          static_cast<float>(new_extent.height),
                          0.f,
                          1.f};

  VkRect2D new_scissor{{0, 0}, new_extent};

  // predeclare to handle failure cleanup
  VkSwapchainKHR new_swapchain{VK_NULL_HANDLE};
  std::vector<VkImage> new_color_images;
  std::vector<VkImageView> new_color_image_views;
  VkImage new_depth_image{VK_NULL_HANDLE}, new_color_target{VK_NULL_HANDLE},
    new_depth_target{VK_NULL_HANDLE};
  VkDeviceMemory new_depth_image_memory{VK_NULL_HANDLE},
    new_color_target_memory{VK_NULL_HANDLE},
    new_depth_target_memory{VK_NULL_HANDLE};
  VkImageView new_depth_image_view{VK_NULL_HANDLE},
    new_color_target_view{VK_NULL_HANDLE},
    new_depth_target_view{VK_NULL_HANDLE};
  std::array<VkImageView, 4> attachments;
  std::vector<VkFramebuffer> new_framebuffers;
  std::vector<VkCommandBuffer> command_buffers;

  new_swapchain =
    ::create_swapchain(_device, s._surface, new_capabilities, new_extent,
                       s._color_format, s._present_mode, s._swapchain, ec);
  if (ec) goto fail;

  std::tie(new_color_images, new_color_image_views) =
    ::get_swapchain_images(_device, new_swapchain, s._color_format.format, ec);
  if (ec) goto fail;

  std::tie(new_depth_image, new_depth_image_memory, new_depth_image_view) =
    ::create_image_and_view(
      _physical, _device, VK_IMAGE_TYPE_2D, s._depth_format, image_extent,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 1, 1,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_SAMPLE_COUNT_1_BIT, 0,
      VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1}, ec);
  if (ec) goto fail;

  transition_depth_image(this, new_depth_image, ec);
  if (ec) goto fail;

  std::tie(new_color_target, new_color_target_memory, new_color_target_view) =
    ::create_image_and_view(_physical, _device, VK_IMAGE_TYPE_2D,
                            s._color_format.format, image_extent,
                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                              VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
                            1, 1, VK_IMAGE_LAYOUT_UNDEFINED, s._samples, 0,
                            VK_IMAGE_VIEW_TYPE_2D,
                            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}, ec);
  if (ec) goto fail;

  std::tie(new_depth_target, new_depth_target_memory, new_depth_target_view) =
    ::create_image_and_view(
      _physical, _device, VK_IMAGE_TYPE_2D, s._depth_format, image_extent,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
      1, 1, VK_IMAGE_LAYOUT_UNDEFINED, s._samples, 0, VK_IMAGE_VIEW_TYPE_2D,
      {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1}, ec);
  if (ec) goto fail;

  attachments[0] = new_color_target_view;
  attachments[2] = new_depth_target_view;
  attachments[3] = new_depth_image_view;

  new_framebuffers =
    create_framebuffers(_device, attachments, new_color_image_views,
                        s._render_pass, new_extent, ec);
  if (ec) goto fail;

  if (s._swapchain != VK_NULL_HANDLE) release(s);

  s._capabilities = new_capabilities;
  s._extent = new_extent;
  s._viewport = new_viewport;
  s._scissor = new_scissor;
  s._swapchain = new_swapchain;
  s._color_images = std::move(new_color_images);
  s._color_image_views = std::move(new_color_image_views);
  s._depth_image = new_depth_image;
  s._depth_image_memory = new_depth_image_memory;
  s._depth_image_view = new_depth_image_view;
  s._color_target = new_color_target;
  s._color_target_memory = new_color_target_memory;
  s._color_target_view = new_color_target_view;
  s._depth_target = new_depth_target;
  s._depth_target_memory = new_depth_target_memory;
  s._depth_target_view = new_depth_target_view;
  s._framebuffers = std::move(new_framebuffers);

  LOG_LEAVE;
  return;

fail:
  for (auto&& framebuffer : new_framebuffers) {
    if (framebuffer != VK_NULL_HANDLE) {
      vkDestroyFramebuffer(_device, framebuffer, nullptr);
    }
  }

  if (new_color_target_view != VK_NULL_HANDLE) {
    vkDestroyImageView(_device, new_color_target_view, nullptr);
  }
  if (new_color_target_memory != VK_NULL_HANDLE) {
    vkFreeMemory(_device, new_color_target_memory, nullptr);
  }
  if (new_color_target != VK_NULL_HANDLE) {
    vkDestroyImage(_device, new_color_target, nullptr);
  }

  if (new_depth_image_view != VK_NULL_HANDLE) {
    vkDestroyImageView(_device, new_depth_image_view, nullptr);
  }
  if (new_depth_image_memory != VK_NULL_HANDLE) {
    vkFreeMemory(_device, new_depth_image_memory, nullptr);
  }
  if (new_depth_image != VK_NULL_HANDLE) {
    vkDestroyImage(_device, new_depth_image, nullptr);
  }

  for (auto&& view : new_color_image_views) {
    if (view != VK_NULL_HANDLE) { vkDestroyImageView(_device, view, nullptr); }
  }

  if (new_swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(_device, new_swapchain, nullptr);
  }

  return;
} // renderer::resize

uint32_t renderer::acquire_next_image(surface& s,
                                      std::error_code& ec) noexcept {
  ec.clear();
  uint32_t image_index;
  VkResult rslt =
    vkAcquireNextImageKHR(_device, s._swapchain, UINT64_MAX, s._image_available,
                          VK_NULL_HANDLE, &image_index);
  if (rslt != VK_SUCCESS) ec.assign(rslt, vk::result_category());
  return image_index;
} // renderer::acquire_next_image

void renderer::submit_present(gsl::span<VkCommandBuffer> buffers, surface& s,
                              uint32_t image_index, VkFence fence,
                              std::error_code& ec) noexcept {
  ec.clear();

  VkPipelineStageFlags const wait_dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
  VkSubmitInfo sinfo = {};
  sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  sinfo.waitSemaphoreCount = 1;
  sinfo.pWaitSemaphores = &s._image_available;
  sinfo.pWaitDstStageMask = &wait_dst;
  sinfo.commandBufferCount = gsl::narrow_cast<uint32_t>(buffers.size());
  sinfo.pCommandBuffers = buffers.data();
  sinfo.signalSemaphoreCount = 1;
  sinfo.pSignalSemaphores = &s._render_finished;

  VkResult rslt = vkQueueSubmit(_graphics_queue, 1, &sinfo, fence);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return;
  }

  VkPresentInfoKHR pinfo = {};
  pinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  pinfo.waitSemaphoreCount = 1;
  pinfo.pWaitSemaphores = &s._render_finished;
  pinfo.swapchainCount = 1;
  pinfo.pSwapchains = &s._swapchain;
  pinfo.pImageIndices = &image_index;

  rslt = vkQueuePresentKHR(_graphics_queue, &pinfo);
  if (rslt != VK_SUCCESS) ec.assign(rslt, vk::result_category());
} // renderer::present

void renderer::destroy(surface& s) noexcept {
  LOG_ENTER;

  release(s);
  if (s._render_pass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(_device, s._render_pass, nullptr);
  }
  if (s._render_finished != VK_NULL_HANDLE) {
    vkDestroySemaphore(_device, s._render_finished, nullptr);
  }
  if (s._image_available != VK_NULL_HANDLE) {
    vkDestroySemaphore(_device, s._image_available, nullptr);
  }
  if (s._surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(_instance, s._surface, nullptr);
  }

  LOG_LEAVE;
} // renderer::destroy

void renderer::release(surface& s) noexcept {
  LOG_ENTER;

  vkDeviceWaitIdle(_device);

  for (auto&& framebuffer : s._framebuffers) {
    vkDestroyFramebuffer(_device, framebuffer, nullptr);
  }
  s._framebuffers.clear();

  if (s._depth_target_view != VK_NULL_HANDLE) {
    vkDestroyImageView(_device, s._depth_target_view, nullptr);
    s._depth_target_view = VK_NULL_HANDLE;
  }
  if (s._depth_target != VK_NULL_HANDLE) {
    vkDestroyImage(_device, s._depth_target, nullptr);
    s._depth_target = VK_NULL_HANDLE;
  }
  if (s._depth_target_memory != VK_NULL_HANDLE) {
    vkFreeMemory(_device, s._depth_target_memory, nullptr);
    s._depth_target_memory = VK_NULL_HANDLE;
  }

  if (s._color_target_view != VK_NULL_HANDLE) {
    vkDestroyImageView(_device, s._color_target_view, nullptr);
    s._color_target_view = VK_NULL_HANDLE;
  }
  if (s._color_target != VK_NULL_HANDLE) {
    vkDestroyImage(_device, s._color_target, nullptr);
    s._color_target = VK_NULL_HANDLE;
  }
  if (s._color_target_memory != VK_NULL_HANDLE) {
    vkFreeMemory(_device, s._color_target_memory, nullptr);
    s._color_target_memory = VK_NULL_HANDLE;
  }

  if (s._depth_image_view != VK_NULL_HANDLE) {
    vkDestroyImageView(_device, s._depth_image_view, nullptr);
    s._depth_image_view = VK_NULL_HANDLE;
  }
  if (s._depth_image != VK_NULL_HANDLE) {
    vkDestroyImage(_device, s._depth_image, nullptr);
    s._depth_image = VK_NULL_HANDLE;
  }
  if (s._depth_image_memory != VK_NULL_HANDLE) {
    vkFreeMemory(_device, s._depth_image_memory, nullptr);
    s._depth_image_memory = VK_NULL_HANDLE;
  }

  for (auto&& view : s._color_image_views) {
    vkDestroyImageView(_device, view, nullptr);
  }
  s._color_image_views.clear();
  s._color_images.clear();

  if (s._swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(_device, s._swapchain, nullptr);
    s._swapchain = VK_NULL_HANDLE;
  }

  LOG_LEAVE;
} // renderer::release

std::vector<VkCommandBuffer>
renderer::allocate_command_buffers(uint32_t count,
                                   std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  std::vector<VkCommandBuffer> command_buffers(count);

  VkCommandBufferAllocateInfo ainfo = {};
  ainfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  ainfo.commandPool = _graphics_command_pool;
  ainfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  ainfo.commandBufferCount = count;

  VkResult rslt =
    vkAllocateCommandBuffers(_device, &ainfo, command_buffers.data());
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return command_buffers;
  }

  LOG_LEAVE;
  return command_buffers;
} // renderer::allocate_command_buffers

void renderer::submit(gsl::span<VkCommandBuffer> command_buffers, bool onetime,
                      std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  VkSubmitInfo sinfo = {};
  sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  sinfo.commandBufferCount = gsl::narrow_cast<uint32_t>(command_buffers.size());
  sinfo.pCommandBuffers = command_buffers.data();

  auto fence = onetime ? _graphics_onetime_fence : VK_NULL_HANDLE;
  VkResult rslt = vkQueueSubmit(_graphics_queue, 1, &sinfo, fence);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return;
  }

  if (onetime) {
    rslt = vkWaitForFences(_device, 1, &_graphics_onetime_fence, VK_TRUE,
                           UINT64_MAX);
    if (rslt != VK_SUCCESS) {
      ec.assign(rslt, vk::result_category());
      return;
    }

    vkResetFences(_device, 1, &_graphics_onetime_fence);
  }

  LOG_LEAVE;
} // renderer::submit

void renderer::free(std::vector<VkCommandBuffer>& command_buffers) noexcept {
  LOG_ENTER;
  if (command_buffers.empty()) {
    LOG_LEAVE;
    return;
  }

  vkDeviceWaitIdle(_device);
  vkFreeCommandBuffers(_device, _graphics_command_pool,
                       gsl::narrow_cast<uint32_t>(command_buffers.size()),
                       command_buffers.data());
  command_buffers.clear();

  LOG_LEAVE;
} // renderer::free

class shader_includer : public shaderc::CompileOptions::IncluderInterface {
public:
  shaderc_include_result* GetInclude(const char* requested_source,
                                     shaderc_include_type type,
                                     const char* requesting_source,
                                     size_t /*include_depth*/) override {
    if (type == shaderc_include_type_relative) {
      plat::filesystem::path parent{requesting_source};
      parent = parent.parent_path();
      _include_paths.push_back(parent / requested_source);
    } else {
      _include_paths.push_back(requested_source);
    }
    plat::filesystem::path& path = _include_paths.back();

    try {
      if (!plat::filesystem::exists(path)) path.clear();
    } catch (...) { path.clear(); }

    if (!path.empty()) {
      std::error_code ec;
      auto source = plat::read_file(path, ec);
      if (ec) {
        _include_sources.push_back(ec.message());
      } else {
        _include_sources.push_back({source.data(), source.size()});
      }
    } else {
      _include_sources.push_back("file not found");
    }

    _include_results.push_back(new shaderc_include_result);
    shaderc_include_result* result = _include_results.back();

    result->source_name_length = _include_paths.back().string().size();
    result->source_name = _include_paths.back().string().c_str();

    result->content_length = _include_sources.back().size();
    result->content = _include_sources.back().data();

    result->user_data = nullptr;
    return result;
  }; // GetInclude

  void ReleaseInclude(shaderc_include_result* data) override {
    for (std::size_t i = 0; i < _include_results.size(); ++i) {
      if (_include_results[i] == data) {
        _include_results.erase(_include_results.begin() + i);
        break;
      }
    }
  } // ReleaseInclude

private:
  std::vector<plat::filesystem::path> _include_paths{};
  std::vector<std::string> _include_sources{};
  std::vector<shaderc_include_result*> _include_results{};
}; // class shader_includer

static std::pair<std::vector<uint32_t>, std::string>
compile_shader(plat::filesystem::path const& path, shaderc_shader_kind kind,
               std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  shaderc::Compiler compiler;
  shaderc::CompileOptions options;
  options.SetOptimizationLevel(shaderc_optimization_level_size);
  options.SetIncluder(gsl::make_unique<shader_includer>());

  auto source = plat::read_file(path, ec);
  if (ec) return {};

  auto spv = compiler.CompileGlslToSpv(source.data(), source.size(), kind,
                                       path.string().c_str(), "main", options);
  if (spv.GetCompilationStatus() != shaderc_compilation_status_success) {
    ec.assign(spv.GetCompilationStatus(), vk::shaderc_result_category());
    return {{}, spv.GetErrorMessage()};
  }

  std::vector<uint32_t> code;
  std::copy(spv.begin(), spv.end(), std::back_inserter(code));

  LOG_LEAVE;
  return {code, ""};
} // compile_shader

shader renderer::create_shader(plat::filesystem::path const& path,
                               shader::types type,
                               std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  shader s;

  auto const kind = [&type]() {
    switch (type) {
    case shader::types::vertex: return shaderc_vertex_shader;
    case shader::types::fragment: return shaderc_fragment_shader;
    default: PLAT_MARK_UNREACHABLE;
    }
  }();

  std::vector<uint32_t> code;
  std::tie(code, s._error_message) = compile_shader(path, kind, ec);
  if (ec) return s;

  VkShaderModuleCreateInfo cinfo = {};
  cinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  cinfo.codeSize = gsl::narrow_cast<uint32_t>(code.size() * 4);
  cinfo.pCode = code.data();

  VkResult rslt = vkCreateShaderModule(_device, &cinfo, nullptr, &s._module);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return s;
  }

  LOG_LEAVE;
  return s;
} // renderer::create_shader

void renderer::destroy(shader& s) noexcept {
  LOG_ENTER;
  if (s._module != VK_NULL_HANDLE) {
    vkDestroyShaderModule(_device, s._module, nullptr);
  }
  s._module = VK_NULL_HANDLE;
  LOG_LEAVE;
} // renderer::destroy

VkPipelineLayout renderer::create_pipeline_layout(
  gsl::span<VkDescriptorSetLayout> descriptor_set_layouts,
  gsl::span<VkPushConstantRange> push_constant_ranges,
  std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  VkPipelineLayoutCreateInfo cinfo = {};
  cinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  cinfo.setLayoutCount =
    gsl::narrow_cast<uint32_t>(descriptor_set_layouts.size());
  cinfo.pSetLayouts = descriptor_set_layouts.data();
  cinfo.pushConstantRangeCount =
    gsl::narrow_cast<uint32_t>(push_constant_ranges.size());
  cinfo.pPushConstantRanges = push_constant_ranges.data();

  VkPipelineLayout layout;
  VkResult rslt = vkCreatePipelineLayout(_device, &cinfo, nullptr, &layout);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_NULL_HANDLE;
  }

  LOG_LEAVE;
  return layout;
} // renderer::create_pipeline_layout

void renderer::destroy(VkPipelineLayout layout) noexcept {
  LOG_ENTER;
  if (layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(_device, layout, nullptr);
  }
  LOG_LEAVE;
} // renderer::destroy

std::vector<VkPipeline>
renderer::create_pipelines(gsl::span<VkGraphicsPipelineCreateInfo> cinfos,
                           std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  std::vector<VkPipeline> pipelines(cinfos.size());
  VkResult rslt = vkCreateGraphicsPipelines(
    _device, VK_NULL_HANDLE, gsl::narrow_cast<uint32_t>(cinfos.size()),
    cinfos.data(), nullptr, pipelines.data());
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return pipelines;
  }

  LOG_LEAVE;
  return pipelines;
} // renderer::create_pipelines

void renderer::destroy(gsl::span<VkPipeline> pipes) noexcept {
  LOG_ENTER;
  for (auto&& pipe : pipes) vkDestroyPipeline(_device, pipe, nullptr);
  LOG_LEAVE;
} // renderer::destroy

VkFence renderer::create_fence(bool signaled, std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  VkFenceCreateInfo fcinfo = {};
  fcinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fcinfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

  VkFence fence;
  VkResult rslt = vkCreateFence(_device, &fcinfo, nullptr, &fence);
  if (rslt != VK_SUCCESS) {
    ec.assign(rslt, vk::result_category());
    return VK_NULL_HANDLE;
  }

  LOG_LEAVE;
  return fence;
} // renderer::create_fence

void renderer::wait(gsl::span<VkFence> fences, bool wait_all, uint64_t timeout,
                    std::error_code& ec) noexcept {
  ec.clear();
  VkResult rslt =
    vkWaitForFences(_device, gsl::narrow_cast<uint32_t>(fences.size()),
                    fences.data(), wait_all, timeout);
  if (rslt != VK_SUCCESS) ec.assign(rslt, vk::result_category());
} // renderer::wait

void renderer::reset(gsl::span<VkFence> fences, std::error_code& ec) noexcept {
  ec.clear();
  VkResult rslt = vkResetFences(
    _device, gsl::narrow_cast<uint32_t>(fences.size()), fences.data());
  if (rslt != VK_SUCCESS) ec.assign(rslt, vk::result_category());
} // renderer::reset

void renderer::destroy(VkFence fence) noexcept {
  LOG_ENTER;
  if (fence != VK_NULL_HANDLE) vkDestroyFence(_device, fence, nullptr);
  LOG_LEAVE;
} // renderer::destroy

renderer::renderer(renderer&& other) noexcept
: _instance{other._instance}
, _callback{other._callback}
, _physical{other._physical}
, _device{other._device}
, _graphics_queue_family_index{other._graphics_queue_family_index}
, _graphics_queue{other._graphics_queue}
, _graphics_command_pool{other._graphics_command_pool}
, _graphics_onetime_fence{other._graphics_onetime_fence} {
  other._instance = VK_NULL_HANDLE;
  other._callback = VK_NULL_HANDLE;
  other._device = VK_NULL_HANDLE;
  other._graphics_command_pool = VK_NULL_HANDLE;
  other._graphics_onetime_fence = VK_NULL_HANDLE;
} // renderer::renderer

renderer& renderer::operator=(renderer&& rhs) noexcept {
  if (this == &rhs) return *this;

  _instance = rhs._instance;
  _callback = rhs._callback;
  _physical = rhs._physical;
  _device = rhs._device;
  _graphics_queue_family_index = rhs._graphics_queue_family_index;
  _graphics_queue = rhs._graphics_queue;
  _graphics_command_pool = rhs._graphics_command_pool;
  _graphics_onetime_fence = rhs._graphics_onetime_fence;

  rhs._instance = VK_NULL_HANDLE;
  rhs._callback = VK_NULL_HANDLE;
  rhs._device = VK_NULL_HANDLE;
  rhs._graphics_command_pool = VK_NULL_HANDLE;
  rhs._graphics_onetime_fence = VK_NULL_HANDLE;

  return *this;
} // renderer::operator=

renderer::~renderer() noexcept {
  LOG_ENTER;

  if (_graphics_onetime_fence != VK_NULL_HANDLE) {
    vkDestroyFence(_device, _graphics_onetime_fence, nullptr);
  }
  if (_graphics_command_pool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(_device, _graphics_command_pool, nullptr);
  }

  if (_device != VK_NULL_HANDLE) vkDestroyDevice(_device, nullptr);
  if (_callback != VK_NULL_HANDLE) {
    vkDestroyDebugReportCallbackEXT(_instance, _callback, nullptr);
  }
  if (_instance != VK_NULL_HANDLE) vkDestroyInstance(_instance, nullptr);

  LOG_LEAVE;
} // renderer::~renderer
