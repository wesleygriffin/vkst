#ifndef VKST_RENDERER_H
#define VKST_RENDERER_H

#include <plat/filesystem.h>
#include <wsi/window.h>
#include <vk/result.h>
#include <gsl.h>
#include <filesystem>
#include <vector>

// Holds all of the data for a surface. This includes the swapchain,
// renderpass, and framebuffers.
// Surfaces must be resized when the window is resized.
class surface {
public:
  std::size_t num_images() const noexcept { return _color_images.size(); }

  VkRenderPass render_pass() const noexcept { return _render_pass; }

  VkFramebuffer framebuffer(std::size_t index) const noexcept {
    return _framebuffers[index];
  }

  VkSampleCountFlagBits samples() const noexcept { return _samples; }

  VkViewport& viewport() noexcept { return _viewport; }
  VkViewport const& viewport() const noexcept { return _viewport; }

  VkRect2D& scissor() noexcept { return _scissor; }
  VkRect2D const& scissor() const noexcept { return _scissor; }

  surface() noexcept {}
  surface(surface const&) = delete;
  surface(surface&& other) noexcept;
  surface& operator=(surface const&) = delete;
  surface& operator=(surface&& rhs) noexcept;
  ~surface() noexcept = default;

private:
  VkSurfaceKHR _surface{VK_NULL_HANDLE};

  VkSurfaceFormatKHR _color_format{};
  VkFormat _depth_format{};
  VkSampleCountFlagBits _samples{};
  VkPresentModeKHR _present_mode{};

  VkSemaphore _image_available{VK_NULL_HANDLE};
  VkSemaphore _render_finished{VK_NULL_HANDLE};
  VkRenderPass _render_pass{VK_NULL_HANDLE};

  VkSurfaceCapabilitiesKHR _capabilities{};
  VkExtent2D _extent{};
  VkViewport _viewport{};
  VkRect2D _scissor{};
  VkSwapchainKHR _swapchain{VK_NULL_HANDLE};

  constexpr static uint32_t MAX_IMAGES = 4;
  std::vector<VkImage> _color_images{};
  std::vector<VkImageView> _color_image_views{};

  VkImage _depth_image{VK_NULL_HANDLE};
  VkDeviceMemory _depth_image_memory{VK_NULL_HANDLE};
  VkImageView _depth_image_view{VK_NULL_HANDLE};

  VkImage _color_target{VK_NULL_HANDLE};
  VkDeviceMemory _color_target_memory{VK_NULL_HANDLE};
  VkImageView _color_target_view{VK_NULL_HANDLE};

  VkImage _depth_target{VK_NULL_HANDLE};
  VkDeviceMemory _depth_target_memory{VK_NULL_HANDLE};
  VkImageView _depth_target_view{VK_NULL_HANDLE};

  std::vector<VkFramebuffer> _framebuffers{};

  friend class renderer;
}; // class surface

// Convenience class to hold both the VkShaderModule for a shader as well as
// any error message from compiling the shader.
class shader {
public:
  enum class types : uint8_t {
    vertex = 0,
    fragment = 1,
  }; // enum class types

  operator VkShaderModule() const noexcept { return _module; }

  std::string const& error_message() const noexcept { return _error_message; }

  shader() noexcept {}
  shader(shader const&) = delete;
  shader(shader&& other) noexcept;
  shader& operator=(shader const&) = delete;
  shader& operator=(shader&& rhs) noexcept;
  ~shader() noexcept = default;

private:
  VkShaderModule _module{VK_NULL_HANDLE};
  std::string _error_message{};

  friend class renderer;
}; // class shader

enum class renderer_result {
  success = 0,
  no_device = 1,
  initialization_failed = 2,
  surface_not_supported = 3,
  no_memory_type = 4,
}; // class renderer_result

class renderer_result_category_impl : public std::error_category {
public:
  virtual char const* name() const noexcept override {
    return "renderer_result";
  }

  virtual std::string message(int ev) const override;
}; // class renderer_result_category_impl

std::error_category const& renderer_result_category();

inline std::error_code make_error_code(renderer_result e) noexcept {
  return {static_cast<int>(e), renderer_result_category()};
}

// Holds all of the data for rendering. Also provides methods for creating
// surfaces, shaders, pipelines, and command buffers, as well as submitting
// command buffers for execution on the device.
class renderer {
public:
  // Create a new renderer. If ec is true, then an error occurred during
  // creation and the renderer object is in an invalid state.
  static renderer create(gsl::czstring application_name,
                         PFN_vkDebugReportCallbackEXT callback,
                         std::error_code& ec) noexcept;

  // Create a new surface. If ec is true, then an error occurred during
  // creation and the surface object is in an invalid state.
  surface create_surface(wsi::window const& window,
                         std::error_code& ec) noexcept;

  // Resize a surface. Must be called when the window that was passed for
  // surface creation is resized. This is not automatically done to allow
  // the render loop to determine when to perform the resize. If ec is true,
  // then an error occurred during the resize.
  void resize(surface& s, wsi::extent2d const& extent,
              std::error_code& ec) noexcept;

  // Acquire the next ready image in the swapchain for rendering to. This
  // must be called and the returned index passed to submit_present. If
  // ec is true, then an error occurred and the returned index is invalid.
  uint32_t acquire_next_image(surface& s, std::error_code& ec) noexcept;

  // Submit a set of command buffers for execution and then present the
  // previously acquired swapchain image. image_index must come from an
  // immediately preceding call to acquire_next_image. fence can be
  // VK_NULL_HANDLE which indicates no fence should be signaled when the
  // command buffers can be reused. If ec is true, then an error occurred
  // during either the submit or present.
  void submit_present(gsl::span<VkCommandBuffer> buffers, surface& s,
                      uint32_t image_index, VkFence fence,
                      std::error_code& ec) noexcept;

  void destroy(surface& s) noexcept;

private:
  void release(surface& s) noexcept;

public:
  // Allocate a set of command buffers. If ec is true, then an error occurred
  // and the vector is invalid.
  std::vector<VkCommandBuffer>
  allocate_command_buffers(uint32_t count, std::error_code& ec) noexcept;

  // Submit a set of command buffers. If ec is true, then an error occurred.
  void submit(std::vector<VkCommandBuffer>& command_buffers, bool onetime,
              std::error_code& ec) noexcept;

  void free(std::vector<VkCommandBuffer>& command_buffers) noexcept;

  // Create a new shader from the given source code. path is expected to hold
  // GLSL source code which will be compiled before creating the shader. If ec
  // is true, then an error occurred and the shader is valid such that
  // shader::error_message can be called to get any compilation errors.
  shader create_shader(plat::filesystem::path const& path, shader::types type,
                       std::error_code& ec) noexcept;

  void destroy(shader& s) noexcept;

  // Create a new pipeline layout. If ec is true, then an error occurred and
  // the pipeline layout object is invalid.
  VkPipelineLayout create_pipeline_layout(
    gsl::span<VkDescriptorSetLayout> descriptor_set_layouts,
    gsl::span<VkPushConstantRange> push_constant_ranges,
    std::error_code& ec) noexcept;

  void destroy(VkPipelineLayout layout) noexcept;

  // Create a new set of pipelines. A single pipeline can also be created.
  // If ec is true, then an error occurred and the vector of pipelines is
  // invalid.
  std::vector<VkPipeline>
  create_pipelines(gsl::span<VkGraphicsPipelineCreateInfo> cinfos,
                   std::error_code& ec) noexcept;

  void destroy(gsl::span<VkPipeline> pipes) noexcept;

  // Create a new fence. If ec is true, then an error occurred and the fence
  // is invalid.
  VkFence create_fence(bool signaled, std::error_code& ec) noexcept;

  // Wait for a set of fences to be signaled. If wait_all is true, then all
  // fences must be signaled before the call will return, if wait_all is
  // false, then a single fence will cause the call to return. timeout
  // specifies how long to wait, 0 returns immediately, and UINT64_MAX will
  // wait indefinitely. If ec is true, and error occurred while waiting.
  void wait(gsl::span<VkFence> fences, bool wait_all, uint64_t timeout,
            std::error_code& ec) noexcept;

  // Reset a set of fences from the signaled state to unsignaled. If ec is
  // true then an error occured.
  void reset(gsl::span<VkFence> fences, std::error_code& ec) noexcept;

  void destroy(VkFence fence) noexcept;

  constexpr renderer() noexcept {};
  renderer(renderer const&) = delete;
  renderer(renderer&& other) noexcept;
  renderer& operator=(renderer const&) = delete;
  renderer& operator=(renderer&& rhs) noexcept;
  ~renderer() noexcept;

private:
  VkInstance _instance{VK_NULL_HANDLE};
  VkDebugReportCallbackEXT _callback{VK_NULL_HANDLE};

  VkPhysicalDevice _physical{VK_NULL_HANDLE};
  VkDevice _device{VK_NULL_HANDLE};

  uint32_t _graphics_queue_index{UINT32_MAX};
  VkQueue _graphics_queue{VK_NULL_HANDLE};
  VkCommandPool _graphics_command_pool{VK_NULL_HANDLE};
  VkFence _graphics_onetime_fence{VK_NULL_HANDLE};
}; // class renderer

#endif // VKST_RENDERER_H
