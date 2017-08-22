#ifndef VK_SUPPORT_H
#define VK_SUPPORT_H

#include <turf/gsl.h>
#include <turf/outcome.h>
#include <wsi/window.h>
#include <vk/result.h>
#include <filesystem>

namespace vk {

outcome::result<VkInstance>
create_instance(gsl::czstring application_name) noexcept;

outcome::result<VkDebugReportCallbackEXT>
create_debug_report_callback(VkInstance instance,
                             PFN_vkDebugReportCallbackEXT callback,
                             void* user_data = nullptr) noexcept;

outcome::result<VkDevice> create_device(VkPhysicalDevice physical) noexcept;

outcome::result<VkCommandPool>
create_command_pool(VkDevice device, uint32_t queue_index) noexcept;

outcome::result<VkFence> create_fence(VkDevice device, bool signaled) noexcept;

outcome::result<VkSurfaceKHR>
create_surface(VkInstance instance, wsi::window const& window) noexcept;

outcome::result<bool> get_surface_support(VkPhysicalDevice physical,
                                          uint32_t queue_index,
                                          VkSurfaceKHR surface) noexcept;

outcome::result<VkSurfaceFormatKHR> choose_surface_format(
  VkPhysicalDevice physical, VkSurfaceKHR surface,
  VkSurfaceFormatKHR desired = {VK_FORMAT_B8G8R8A8_UNORM,
                                VK_COLORSPACE_SRGB_NONLINEAR_KHR}) noexcept;

outcome::result<VkPresentModeKHR> choose_present_mode(
  VkPhysicalDevice physical, VkSurfaceKHR surface,
  VkPresentModeKHR desired = VK_PRESENT_MODE_FIFO_KHR) noexcept;

outcome::result<VkImage>
create_image(VkDevice device, VkImageType type, VkFormat format,
             VkExtent3D extent, VkImageUsageFlags usage,
             uint32_t mip_levels = 1, uint32_t array_layers = 1,
             VkImageLayout initial = VK_IMAGE_LAYOUT_UNDEFINED,
             VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
             VkImageCreateFlags flags = 0) noexcept;

outcome::result<VkImageView>
create_image_view(VkDevice device, VkImage image, VkImageViewType type,
                  VkFormat format, VkImageSubresourceRange isr) noexcept;

outcome::result<int32_t>
find_memory_type(VkPhysicalDeviceMemoryProperties memory_properties,
                 uint32_t memory_type_bits,
                 VkMemoryPropertyFlags properties) noexcept;

outcome::result<VkDeviceMemory>
allocate_memory(VkPhysicalDevice physical, VkDevice device,
                VkMemoryRequirements requirements,
                VkMemoryPropertyFlags flags) noexcept;

outcome::result<VkDeviceMemory>
allocate_memory(VkPhysicalDevice physical, VkDevice device, VkImage image,
                VkMemoryPropertyFlags flags) noexcept;

outcome::result<VkDeviceMemory>
allocate_memory(VkPhysicalDevice physical, VkDevice device, VkBuffer buffer,
                VkMemoryPropertyFlags flags) noexcept;

outcome::result<VkFramebuffer>
create_framebuffer(VkDevice device, std::size_t num_attachments,
                   VkImageView* attachments, VkRenderPass render_pass,
                   uint32_t width, uint32_t height) noexcept;

outcome::result<VkSemaphore> create_semaphore(VkDevice device) noexcept;

outcome::result<std::vector<uint32_t>>
compile_shader(std::experimental::filesystem::path const& path,
               shaderc_shader_kind kind) noexcept;

outcome::result<VkShaderModule>
create_shader_module(VkDevice device, gsl::span<uint32_t> code) noexcept;

} // namespace vk

#endif // VK_SUPPORT_H