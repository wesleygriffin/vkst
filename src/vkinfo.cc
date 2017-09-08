#include <plat/core.h>
#include <wsi/window.h>
#include <vk/result.h>
#include <string>
#include <vector>
using namespace std::string_literals;

FILE* s_file;

std::string to_string(VkPhysicalDeviceType type) {
  switch (type) {
  case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "Other";
  case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
  case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "Discrete GPU";
  case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "Virtual GPU";
  case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
  }
  PLAT_MARK_UNREACHABLE;
}

template <class B>
std::string to_string(B flags, B bit, gsl::czstring s) {
  if ((flags & bit) == 0) return "";
  if ((flags ^ bit) != 0) return s + " | "s;
  return s + " "s;
}

std::string to_string(VkQueueFlagBits flags) {
  auto str = to_string(flags, VK_QUEUE_GRAPHICS_BIT, "Graphics") +
             to_string(flags, VK_QUEUE_COMPUTE_BIT, "Compute") +
             to_string(flags, VK_QUEUE_TRANSFER_BIT, "Transfer") +
             to_string(flags, VK_QUEUE_SPARSE_BINDING_BIT, "SparseBinding");
  if (!str.empty() && str[str.size() - 2] == '|') str.erase(str.size() - 2);
  return "{ " + str + "}";
}

std::string to_string(VkMemoryPropertyFlagBits flags) {
  auto str =
    to_string(flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "DeviceLocal") +
    to_string(flags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, "HostVisible") +
    to_string(flags, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "HostCoherent") +
    to_string(flags, VK_MEMORY_PROPERTY_HOST_CACHED_BIT, "HostCached") +
    to_string(flags, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
              "LazilyAllocated");
  if (!str.empty() && str[str.size() - 2] == '|') str.erase(str.size() - 2);
  return "{ " + str + "}";
}

std::string to_string(VkMemoryHeapFlagBits flags) {
  auto str =
    to_string(flags, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, "DeviceLocal") +
    to_string(flags, VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHX, "MultiInstanceKHX");
  if (!str.empty() && str[str.size() - 2] == '|') str.erase(str.size() - 2);
  return "{ " + str + "}";
}

std::string to_string(VkSurfaceTransformFlagBitsKHR flags) {
  auto str =
    to_string(flags, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, "Identity") +
    to_string(flags, VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR, "Rotate90") +
    to_string(flags, VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR, "Rotate180") +
    to_string(flags, VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR, "Rotate270") +
    to_string(flags, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR,
              "HorizontalMirror") +
    to_string(flags, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR,
              "HorizontalMirrorRotate90") +
    to_string(flags, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR,
              "HorizontalMirrorRotate180") +
    to_string(flags, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR,
              "HorizontalMirrorRotate270") +
    to_string(flags, VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR, "Inherit");
  if (!str.empty() && str[str.size() - 2] == '|') str.erase(str.size() - 2);
  return "{ " + str + "}";
}

std::string to_string(VkCompositeAlphaFlagBitsKHR flags) {
  auto str = to_string(flags, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, "Opaque") +
             to_string(flags, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                       "PreMultiplied") +
             to_string(flags, VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
                       "PostMultiplied") +
             to_string(flags, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, "Inherit");
  if (!str.empty() && str[str.size() - 2] == '|') str.erase(str.size() - 2);
  return "{ " + str + "}";
}

std::string to_string(VkImageUsageFlagBits flags) {
  auto str =
    to_string(flags, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, "TransferSrc") +
    to_string(flags, VK_IMAGE_USAGE_TRANSFER_DST_BIT, "TransferDst") +
    to_string(flags, VK_IMAGE_USAGE_SAMPLED_BIT, "Sampled") +
    to_string(flags, VK_IMAGE_USAGE_STORAGE_BIT, "Storage") +
    to_string(flags, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "ColorAttachment") +
    to_string(flags, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
              "DepthStencilAttachment") +
    to_string(flags, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
              "TransientAttachment") +
    to_string(flags, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, "InputAttachment");
  if (!str.empty() && str[str.size() - 2] == '|') str.erase(str.size() - 2);
  return "{ " + str + "}";
}

std::string to_string(VkDebugReportFlagBitsEXT flags) {
  auto str =
    to_string(flags, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, "Information") +
    to_string(flags, VK_DEBUG_REPORT_WARNING_BIT_EXT, "Warning") +
    to_string(flags, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
              "PerformanceWarning") +
    to_string(flags, VK_DEBUG_REPORT_ERROR_BIT_EXT, "Error") +
    to_string(flags, VK_DEBUG_REPORT_DEBUG_BIT_EXT, "Debug");
  if (!str.empty() && str[str.size() - 2] == '|') str.erase(str.size() - 2);
  return "{ " + str + "}";
}

std::string to_string(VkFormat format) {
  switch (format) {
  case VK_FORMAT_UNDEFINED: return "UNDEFINED";
  case VK_FORMAT_R4G4_UNORM_PACK8: return "R4G4_UNORM_PACK8";
  case VK_FORMAT_R4G4B4A4_UNORM_PACK16: return "R4G4B4A4_UNORM_PACK16";
  case VK_FORMAT_B4G4R4A4_UNORM_PACK16: return "B4G4R4A4_UNORM_PACK16";
  case VK_FORMAT_R5G6B5_UNORM_PACK16: return "R5G6B5_UNORM_PACK16";
  case VK_FORMAT_B5G6R5_UNORM_PACK16: return "B5G6R5_UNORM_PACK16";
  case VK_FORMAT_R5G5B5A1_UNORM_PACK16: return "R5G5B5A1_UNORM_PACK16";
  case VK_FORMAT_B5G5R5A1_UNORM_PACK16: return "B5G5R5A1_UNORM_PACK16";
  case VK_FORMAT_A1R5G5B5_UNORM_PACK16: return "A1R5G5B5_UNORM_PACK16";
  case VK_FORMAT_R8_UNORM: return "R8_UNORM";
  case VK_FORMAT_R8_SNORM: return "R8_SNORM";
  case VK_FORMAT_R8_USCALED: return "R8_USCALED";
  case VK_FORMAT_R8_SSCALED: return "R8_SSCALED";
  case VK_FORMAT_R8_UINT: return "R8_UINT";
  case VK_FORMAT_R8_SINT: return "R8_SINT";
  case VK_FORMAT_R8_SRGB: return "R8_SRGB";
  case VK_FORMAT_R8G8_UNORM: return "R8G8_UNORM";
  case VK_FORMAT_R8G8_SNORM: return "R8G8_SNORM";
  case VK_FORMAT_R8G8_USCALED: return "R8G8_USCALED";
  case VK_FORMAT_R8G8_SSCALED: return "R8G8_SSCALED";
  case VK_FORMAT_R8G8_UINT: return "R8G8_UINT";
  case VK_FORMAT_R8G8_SINT: return "R8G8_SINT";
  case VK_FORMAT_R8G8_SRGB: return "R8G8_SRGB";
  case VK_FORMAT_R8G8B8_UNORM: return "R8G8B8_UNORM";
  case VK_FORMAT_R8G8B8_SNORM: return "R8G8B8_SNORM";
  case VK_FORMAT_R8G8B8_USCALED: return "R8G8B8_USCALED";
  case VK_FORMAT_R8G8B8_SSCALED: return "R8G8B8_SSCALED";
  case VK_FORMAT_R8G8B8_UINT: return "R8G8B8_UINT";
  case VK_FORMAT_R8G8B8_SINT: return "R8G8B8_SINT";
  case VK_FORMAT_R8G8B8_SRGB: return "R8G8B8_SRGB";
  case VK_FORMAT_B8G8R8_UNORM: return "B8G8R8_UNORM";
  case VK_FORMAT_B8G8R8_SNORM: return "B8G8R8_SNORM";
  case VK_FORMAT_B8G8R8_USCALED: return "B8G8R8_USCALED";
  case VK_FORMAT_B8G8R8_SSCALED: return "B8G8R8_SSCALED";
  case VK_FORMAT_B8G8R8_UINT: return "B8G8R8_UINT";
  case VK_FORMAT_B8G8R8_SINT: return "B8G8R8_SINT";
  case VK_FORMAT_B8G8R8_SRGB: return "B8G8R8_SRGB";
  case VK_FORMAT_R8G8B8A8_UNORM: return "R8G8B8A8_UNORM";
  case VK_FORMAT_R8G8B8A8_SNORM: return "R8G8B8A8_SNORM";
  case VK_FORMAT_R8G8B8A8_USCALED: return "R8G8B8A8_USCALED";
  case VK_FORMAT_R8G8B8A8_SSCALED: return "R8G8B8A8_SSCALED";
  case VK_FORMAT_R8G8B8A8_UINT: return "R8G8B8A8_UINT";
  case VK_FORMAT_R8G8B8A8_SINT: return "R8G8B8A8_SINT";
  case VK_FORMAT_R8G8B8A8_SRGB: return "R8G8B8A8_SRGB";
  case VK_FORMAT_B8G8R8A8_UNORM: return "B8G8R8A8_UNORM";
  case VK_FORMAT_B8G8R8A8_SNORM: return "B8G8R8A8_SNORM";
  case VK_FORMAT_B8G8R8A8_USCALED: return "B8G8R8A8_USCALED";
  case VK_FORMAT_B8G8R8A8_SSCALED: return "B8G8R8A8_SSCALED";
  case VK_FORMAT_B8G8R8A8_UINT: return "B8G8R8A8_UINT";
  case VK_FORMAT_B8G8R8A8_SINT: return "B8G8R8A8_SINT";
  case VK_FORMAT_B8G8R8A8_SRGB: return "B8G8R8A8_SRGB";
  case VK_FORMAT_A8B8G8R8_UNORM_PACK32: return "A8B8G8R8_UNORM_PACK32";
  case VK_FORMAT_A8B8G8R8_SNORM_PACK32: return "A8B8G8R8_SNORM_PACK32";
  case VK_FORMAT_A8B8G8R8_USCALED_PACK32: return "A8B8G8R8_USCALED_PACK32";
  case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: return "A8B8G8R8_SSCALED_PACK32";
  case VK_FORMAT_A8B8G8R8_UINT_PACK32: return "A8B8G8R8_UINT_PACK32";
  case VK_FORMAT_A8B8G8R8_SINT_PACK32: return "A8B8G8R8_SINT_PACK32";
  case VK_FORMAT_A8B8G8R8_SRGB_PACK32: return "A8B8G8R8_SRGB_PACK32";
  case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return "A2R10G10B10_UNORM_PACK32";
  case VK_FORMAT_A2R10G10B10_SNORM_PACK32: return "A2R10G10B10_SNORM_PACK32";
  case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
    return "A2R10G10B10_USCALED_PACK32";
  case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
    return "A2R10G10B10_SSCALED_PACK32";
  case VK_FORMAT_A2R10G10B10_UINT_PACK32: return "A2R10G10B10_UINT_PACK32";
  case VK_FORMAT_A2R10G10B10_SINT_PACK32: return "A2R10G10B10_SINT_PACK32";
  case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return "A2B10G10R10_UNORM_PACK32";
  case VK_FORMAT_A2B10G10R10_SNORM_PACK32: return "A2B10G10R10_SNORM_PACK32";
  case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
    return "A2B10G10R10_USCALED_PACK32";
  case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
    return "A2B10G10R10_SSCALED_PACK32";
  case VK_FORMAT_A2B10G10R10_UINT_PACK32: return "A2B10G10R10_UINT_PACK32";
  case VK_FORMAT_A2B10G10R10_SINT_PACK32: return "A2B10G10R10_SINT_PACK32";
  case VK_FORMAT_R16_UNORM: return "R16_UNORM";
  case VK_FORMAT_R16_SNORM: return "R16_SNORM";
  case VK_FORMAT_R16_USCALED: return "R16_USCALED";
  case VK_FORMAT_R16_SSCALED: return "R16_SSCALED";
  case VK_FORMAT_R16_UINT: return "R16_UINT";
  case VK_FORMAT_R16_SINT: return "R16_SINT";
  case VK_FORMAT_R16_SFLOAT: return "R16_SFLOAT";
  case VK_FORMAT_R16G16_UNORM: return "R16G16_UNORM";
  case VK_FORMAT_R16G16_SNORM: return "R16G16_SNORM";
  case VK_FORMAT_R16G16_USCALED: return "R16G16_USCALED";
  case VK_FORMAT_R16G16_SSCALED: return "R16G16_SSCALED";
  case VK_FORMAT_R16G16_UINT: return "R16G16_UINT";
  case VK_FORMAT_R16G16_SINT: return "R16G16_SINT";
  case VK_FORMAT_R16G16_SFLOAT: return "R16G16_SFLOAT";
  case VK_FORMAT_R16G16B16_UNORM: return "R16G16B16_UNORM";
  case VK_FORMAT_R16G16B16_SNORM: return "R16G16B16_SNORM";
  case VK_FORMAT_R16G16B16_USCALED: return "R16G16B16_USCALED";
  case VK_FORMAT_R16G16B16_SSCALED: return "R16G16B16_SSCALED";
  case VK_FORMAT_R16G16B16_UINT: return "R16G16B16_UINT";
  case VK_FORMAT_R16G16B16_SINT: return "R16G16B16_SINT";
  case VK_FORMAT_R16G16B16_SFLOAT: return "R16G16B16_SFLOAT";
  case VK_FORMAT_R16G16B16A16_UNORM: return "R16G16B16A16_UNORM";
  case VK_FORMAT_R16G16B16A16_SNORM: return "R16G16B16A16_SNORM";
  case VK_FORMAT_R16G16B16A16_USCALED: return "R16G16B16A16_USCALED";
  case VK_FORMAT_R16G16B16A16_SSCALED: return "R16G16B16A16_SSCALED";
  case VK_FORMAT_R16G16B16A16_UINT: return "R16G16B16A16_UINT";
  case VK_FORMAT_R16G16B16A16_SINT: return "R16G16B16A16_SINT";
  case VK_FORMAT_R16G16B16A16_SFLOAT: return "R16G16B16A16_SFLOAT";
  case VK_FORMAT_R32_UINT: return "R32_UINT";
  case VK_FORMAT_R32_SINT: return "R32_SINT";
  case VK_FORMAT_R32_SFLOAT: return "R32_SFLOAT";
  case VK_FORMAT_R32G32_UINT: return "R32G32_UINT";
  case VK_FORMAT_R32G32_SINT: return "R32G32_SINT";
  case VK_FORMAT_R32G32_SFLOAT: return "R32G32_SFLOAT";
  case VK_FORMAT_R32G32B32_UINT: return "R32G32B32_UINT";
  case VK_FORMAT_R32G32B32_SINT: return "R32G32B32_SINT";
  case VK_FORMAT_R32G32B32_SFLOAT: return "R32G32B32_SFLOAT";
  case VK_FORMAT_R32G32B32A32_UINT: return "R32G32B32A32_UINT";
  case VK_FORMAT_R32G32B32A32_SINT: return "R32G32B32A32_SINT";
  case VK_FORMAT_R32G32B32A32_SFLOAT: return "R32G32B32A32_SFLOAT";
  case VK_FORMAT_R64_UINT: return "R64_UINT";
  case VK_FORMAT_R64_SINT: return "R64_SINT";
  case VK_FORMAT_R64_SFLOAT: return "R64_SFLOAT";
  case VK_FORMAT_R64G64_UINT: return "R64G64_UINT";
  case VK_FORMAT_R64G64_SINT: return "R64G64_SINT";
  case VK_FORMAT_R64G64_SFLOAT: return "R64G64_SFLOAT";
  case VK_FORMAT_R64G64B64_UINT: return "R64G64B64_UINT";
  case VK_FORMAT_R64G64B64_SINT: return "R64G64B64_SINT";
  case VK_FORMAT_R64G64B64_SFLOAT: return "R64G64B64_SFLOAT";
  case VK_FORMAT_R64G64B64A64_UINT: return "R64G64B64A64_UINT";
  case VK_FORMAT_R64G64B64A64_SINT: return "R64G64B64A64_SINT";
  case VK_FORMAT_R64G64B64A64_SFLOAT: return "R64G64B64A64_SFLOAT";
  case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return "B10G11R11_UFLOAT_PACK32";
  case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return "E5B9G9R9_UFLOAT_PACK32";
  case VK_FORMAT_D16_UNORM: return "D16_UNORM";
  case VK_FORMAT_X8_D24_UNORM_PACK32: return "X8_D24_UNORM_PACK32";
  case VK_FORMAT_D32_SFLOAT: return "D32_SFLOAT";
  case VK_FORMAT_S8_UINT: return "S8_UINT";
  case VK_FORMAT_D16_UNORM_S8_UINT: return "D16_UNORM_S8_UINT";
  case VK_FORMAT_D24_UNORM_S8_UINT: return "D24_UNORM_S8_UINT";
  case VK_FORMAT_D32_SFLOAT_S8_UINT: return "D32_SFLOAT_S8_UINT";
  case VK_FORMAT_BC1_RGB_UNORM_BLOCK: return "BC1_RGB_UNORM_BLOCK";
  case VK_FORMAT_BC1_RGB_SRGB_BLOCK: return "BC1_RGB_SRGB_BLOCK";
  case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: return "BC1_RGBA_UNORM_BLOCK";
  case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return "BC1_RGBA_SRGB_BLOCK";
  case VK_FORMAT_BC2_UNORM_BLOCK: return "BC2_UNORM_BLOCK";
  case VK_FORMAT_BC2_SRGB_BLOCK: return "BC2_SRGB_BLOCK";
  case VK_FORMAT_BC3_UNORM_BLOCK: return "BC3_UNORM_BLOCK";
  case VK_FORMAT_BC3_SRGB_BLOCK: return "BC3_SRGB_BLOCK";
  case VK_FORMAT_BC4_UNORM_BLOCK: return "BC4_UNORM_BLOCK";
  case VK_FORMAT_BC4_SNORM_BLOCK: return "BC4_SNORM_BLOCK";
  case VK_FORMAT_BC5_UNORM_BLOCK: return "BC5_UNORM_BLOCK";
  case VK_FORMAT_BC5_SNORM_BLOCK: return "BC5_SNORM_BLOCK";
  case VK_FORMAT_BC6H_UFLOAT_BLOCK: return "BC6H_UFLOAT_BLOCK";
  case VK_FORMAT_BC6H_SFLOAT_BLOCK: return "BC6H_SFLOAT_BLOCK";
  case VK_FORMAT_BC7_UNORM_BLOCK: return "BC7_UNORM_BLOCK";
  case VK_FORMAT_BC7_SRGB_BLOCK: return "BC7_SRGB_BLOCK";
  case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return "ETC2_R8G8B8_UNORM_BLOCK";
  case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return "ETC2_R8G8B8_SRGB_BLOCK";
  case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: return "ETC2_R8G8B8A1_UNORM_BLOCK";
  case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return "ETC2_R8G8B8A1_SRGB_BLOCK";
  case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK: return "ETC2_R8G8B8A8_UNORM_BLOCK";
  case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: return "ETC2_R8G8B8A8_SRGB_BLOCK";
  case VK_FORMAT_EAC_R11_UNORM_BLOCK: return "EAC_R11_UNORM_BLOCK";
  case VK_FORMAT_EAC_R11_SNORM_BLOCK: return "EAC_R11_SNORM_BLOCK";
  case VK_FORMAT_EAC_R11G11_UNORM_BLOCK: return "EAC_R11G11_UNORM_BLOCK";
  case VK_FORMAT_EAC_R11G11_SNORM_BLOCK: return "EAC_R11G11_SNORM_BLOCK";
  case VK_FORMAT_ASTC_4x4_UNORM_BLOCK: return "ASTC_4x4_UNORM_BLOCK";
  case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: return "ASTC_4x4_SRGB_BLOCK";
  case VK_FORMAT_ASTC_5x4_UNORM_BLOCK: return "ASTC_5x4_UNORM_BLOCK";
  case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: return "ASTC_5x4_SRGB_BLOCK";
  case VK_FORMAT_ASTC_5x5_UNORM_BLOCK: return "ASTC_5x5_UNORM_BLOCK";
  case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: return "ASTC_5x5_SRGB_BLOCK";
  case VK_FORMAT_ASTC_6x5_UNORM_BLOCK: return "ASTC_6x5_UNORM_BLOCK";
  case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: return "ASTC_6x5_SRGB_BLOCK";
  case VK_FORMAT_ASTC_6x6_UNORM_BLOCK: return "ASTC_6x6_UNORM_BLOCK";
  case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: return "ASTC_6x6_SRGB_BLOCK";
  case VK_FORMAT_ASTC_8x5_UNORM_BLOCK: return "ASTC_8x5_UNORM_BLOCK";
  case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: return "ASTC_8x5_SRGB_BLOCK";
  case VK_FORMAT_ASTC_8x6_UNORM_BLOCK: return "ASTC_8x6_UNORM_BLOCK";
  case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: return "ASTC_8x6_SRGB_BLOCK";
  case VK_FORMAT_ASTC_8x8_UNORM_BLOCK: return "ASTC_8x8_UNORM_BLOCK";
  case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: return "ASTC_8x8_SRGB_BLOCK";
  case VK_FORMAT_ASTC_10x5_UNORM_BLOCK: return "ASTC_10x5_UNORM_BLOCK";
  case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: return "ASTC_10x5_SRGB_BLOCK";
  case VK_FORMAT_ASTC_10x6_UNORM_BLOCK: return "ASTC_10x6_UNORM_BLOCK";
  case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: return "ASTC_10x6_SRGB_BLOCK";
  case VK_FORMAT_ASTC_10x8_UNORM_BLOCK: return "ASTC_10x8_UNORM_BLOCK";
  case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: return "ASTC_10x8_SRGB_BLOCK";
  case VK_FORMAT_ASTC_10x10_UNORM_BLOCK: return "ASTC_10x10_UNORM_BLOCK";
  case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: return "ASTC_10x10_SRGB_BLOCK";
  case VK_FORMAT_ASTC_12x10_UNORM_BLOCK: return "ASTC_12x10_UNORM_BLOCK";
  case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: return "ASTC_12x10_SRGB_BLOCK";
  case VK_FORMAT_ASTC_12x12_UNORM_BLOCK: return "ASTC_12x12_UNORM_BLOCK";
  case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: return "ASTC_12x12_SRGB_BLOCK";
  case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
    return "PVRTC1_2BPP_UNORM_BLOCK_IMG";
  case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
    return "PVRTC1_4BPP_UNORM_BLOCK_IMG";
  case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
    return "PVRTC2_2BPP_UNORM_BLOCK_IMG";
  case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
    return "PVRTC2_4BPP_UNORM_BLOCK_IMG";
  case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
    return "PVRTC1_2BPP_SRGB_BLOCK_IMG";
  case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
    return "PVRTC1_4BPP_SRGB_BLOCK_IMG";
  case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
    return "PVRTC2_2BPP_SRGB_BLOCK_IMG";
  case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
    return "PVRTC2_4BPP_SRGB_BLOCK_IMG";
  }
  PLAT_MARK_UNREACHABLE;
}

std::string to_string(VkColorSpaceKHR color_space) {
  switch (color_space) {
  case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: return "SRGB_NONLINEAR_KHR";
  case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
    return "DISPLAY_P3_NONLINEAR_EXT";
  case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
    return "EXTENDED_SRGB_LINEAR_EXT";
  case VK_COLOR_SPACE_DCI_P3_LINEAR_EXT: return "DCI_P3_LINEAR_EXT";
  case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT: return "DCI_P3_NONLINEAR_EXT";
  case VK_COLOR_SPACE_BT709_LINEAR_EXT: return "BT709_LINEAR_EXT";
  case VK_COLOR_SPACE_BT709_NONLINEAR_EXT: return "BT709_NONLINEAR_EXT";
  case VK_COLOR_SPACE_BT2020_LINEAR_EXT: return "BT2020_LINEAR_EXT";
  case VK_COLOR_SPACE_HDR10_ST2084_EXT: return "HDR10_ST2084_EXT";
  case VK_COLOR_SPACE_DOLBYVISION_EXT: return "DOLBYVISION_EXT";
  case VK_COLOR_SPACE_HDR10_HLG_EXT: return "HDR10_HLG_EXT";
  case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT: return "ADOBERGB_LINEAR_EXT";
  case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT: return "ADOBERGB_NONLINEAR_EXT";
  case VK_COLOR_SPACE_PASS_THROUGH_EXT: return "PASS_THROUGH_EXT";
  }
  PLAT_MARK_UNREACHABLE;
}

std::error_code dump_instance_layers() {
  uint32_t num;
  VkResult rslt = vkEnumerateInstanceLayerProperties(&num, nullptr);
  if (rslt != VK_SUCCESS) return vk::make_error_code(rslt);

  std::vector<VkLayerProperties> props(num);
  rslt = vkEnumerateInstanceLayerProperties(&num, props.data());
  if (rslt != VK_SUCCESS) return vk::make_error_code(rslt);

  std::fprintf(s_file, "%u instance layer properties:\n", num);
  for (auto&& prop : props) {
    std::fprintf(s_file, "    %s (%d.%d.%d)\n", prop.layerName,
                 VK_VERSION_MAJOR(prop.specVersion),
                 VK_VERSION_MINOR(prop.specVersion),
                 VK_VERSION_PATCH(prop.specVersion));
  }

  return vk::make_error_code(vk::result::success);
}

std::error_code dump_instance_extensions() {
  uint32_t num;
  VkResult rslt = vkEnumerateInstanceExtensionProperties("", &num, nullptr);
  if (rslt != VK_SUCCESS) return vk::make_error_code(rslt);

  std::vector<VkExtensionProperties> props(num);
  rslt = vkEnumerateInstanceExtensionProperties("", &num, props.data());
  if (rslt != VK_SUCCESS) return vk::make_error_code(rslt);

  std::fprintf(s_file, "%u instance extensions properties:\n", num);
  for (auto&& prop : props) {
    std::fprintf(s_file, "    %s (%d.%d.%d)\n", prop.extensionName,
                 VK_VERSION_MAJOR(prop.specVersion),
                 VK_VERSION_MINOR(prop.specVersion),
                 VK_VERSION_PATCH(prop.specVersion));
  }

  return vk::make_error_code(vk::result::success);
}

static VkBool32 debug_report(VkDebugReportFlagsEXT flags,
                             VkDebugReportObjectTypeEXT, uint64_t, size_t,
                             int32_t, char const* layer_prefix,
                             char const* message, void*) noexcept {
  if ((flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) ==
      VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
    return VK_FALSE;
  }

  std::fprintf(s_file, "%s %s: %s\n", layer_prefix,
               to_string(static_cast<VkDebugReportFlagBitsEXT>(flags)).c_str(),
               message);
  return VK_FALSE;
}

VkInstance create_instance(std::error_code& ec) {
  std::vector<char const*> enabled_layer_names{
    "VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation",
    "VK_LAYER_LUNARG_object_tracker", "VK_LAYER_LUNARG_core_validation"};
  std::vector<char const*> enabled_extension_names{
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME};
#if TURF_TARGET_WIN32
  enabled_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif TURF_KERNEL_LINUX
  enabled_extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

  VkApplicationInfo application_info;
  application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  application_info.pNext = nullptr;
  application_info.pApplicationName = "vkinfo";
  application_info.applicationVersion = 0;
  application_info.pEngineName = nullptr;
  application_info.engineVersion = 0;
  application_info.apiVersion = VK_MAKE_VERSION(1, 0, 54);

  VkInstanceCreateInfo create_info;
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.pApplicationInfo = &application_info;
  create_info.enabledLayerCount =
    gsl::narrow_cast<uint32_t>(enabled_layer_names.size());
  create_info.ppEnabledLayerNames = enabled_layer_names.data();
  create_info.enabledExtensionCount =
    gsl::narrow_cast<uint32_t>(enabled_extension_names.size());
  create_info.ppEnabledExtensionNames = enabled_extension_names.data();

  VkDebugReportCallbackCreateInfoEXT debug_report_callback_create_info;
  debug_report_callback_create_info.sType =
    VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  debug_report_callback_create_info.pNext = nullptr;
  debug_report_callback_create_info.flags =
    VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;
  debug_report_callback_create_info.pfnCallback = &debug_report;
  debug_report_callback_create_info.pUserData = nullptr;

  create_info.pNext = &debug_report_callback_create_info;

  VkInstance instance;
  VkResult rslt = vkCreateInstance(&create_info, nullptr, &instance);
  ec.assign(rslt, vk::result_category());
  return instance;
}

VkDebugReportCallbackEXT create_debug_report_callback(VkInstance instance,
                                                      std::error_code& ec) {
  auto createDebugReportCallbackEXT =
    reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
      vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
  if (!createDebugReportCallbackEXT) {
    ec.assign(static_cast<int>(vk::result::error_extension_not_present),
              vk::result_category());
    return VK_NULL_HANDLE;
  }

  VkDebugReportCallbackCreateInfoEXT create_info;
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  create_info.pNext = nullptr;
  create_info.flags = VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;
  create_info.pfnCallback = &debug_report;
  create_info.pUserData = nullptr;

  VkDebugReportCallbackEXT debug_report_callback;
  VkResult rslt = createDebugReportCallbackEXT(instance, &create_info, nullptr,
                                               &debug_report_callback);
  ec.assign(rslt, vk::result_category());
  return debug_report_callback;
}

VkSurfaceKHR create_surface(VkInstance instance, wsi::window const& win,
                            std::error_code& ec) {
  VkResult rslt;
  VkSurfaceKHR sfc;
  auto native = win.native_handle();

#if TURF_TARGET_WIN32

  VkWin32SurfaceCreateInfoKHR create_info;
  create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.hinstance = native.first;
  create_info.hwnd = native.second;

  rslt = vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &sfc);

#elif TURF_KERNEL_LINUX

  VkXlibSurfaceCreateInfoKHR create_info;
  create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.dpy = std::get<0>(native);
  create_info.window = std::get<2>(native);

  rslt = vkCreateXlibSurfaceKHR(instance, &create_info, nullptr, &sfc);

#endif

  ec.assign(rslt, vk::result_category());
  return sfc;
}

std::error_code dump_physical_device(VkInstance instance,
                                     VkPhysicalDevice device,
                                     VkSurfaceKHR surface) {
  auto getPhysicalDeviceProperties2KHR =
    reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(
      vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2KHR"));
  if (!getPhysicalDeviceProperties2KHR) {
    return vk::make_error_code(vk::result::error_extension_not_present);
  }

  auto getPhysicalDeviceFeatures2KHR =
    reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
      vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures2KHR"));
  if (!getPhysicalDeviceFeatures2KHR) {
    return vk::make_error_code(vk::result::error_extension_not_present);
  }

  auto getPhysicalDeviceQueueFamilyProperties2KHR =
    reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR>(
      vkGetInstanceProcAddr(instance,
                            "vkGetPhysicalDeviceQueueFamilyProperties2KHR"));
  if (!getPhysicalDeviceQueueFamilyProperties2KHR) {
    return vk::make_error_code(vk::result::error_extension_not_present);
  }

  auto getPhysicalDeviceMemoryProperties2KHR =
    reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2KHR>(
      vkGetInstanceProcAddr(instance,
                            "vkGetPhysicalDeviceMemoryProperties2KHR"));
  if (!getPhysicalDeviceMemoryProperties2KHR) {
    return vk::make_error_code(vk::result::error_extension_not_present);
  }

  auto getPhysicalDeviceSurfaceCapabilities2KHR =
    reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR>(
      vkGetInstanceProcAddr(instance,
                            "vkGetPhysicalDeviceSurfaceCapabilities2KHR"));
  if (!getPhysicalDeviceSurfaceCapabilities2KHR) {
    return vk::make_error_code(vk::result::error_extension_not_present);
  }

  auto getPhysicalDeviceSurfaceFormats2KHR =
    reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormats2KHR>(
      vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormats2KHR"));
  if (!getPhysicalDeviceSurfaceFormats2KHR) {
    return vk::make_error_code(vk::result::error_extension_not_present);
  }

  VkPhysicalDeviceMultiviewPropertiesKHX mp;
  mp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES_KHX;
  mp.pNext = nullptr;

  VkPhysicalDeviceIDPropertiesKHR id;
  id.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES_KHR;
  id.pNext = &mp;

  VkPhysicalDeviceProperties2KHR props;
  props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
  props.pNext = &id;

  getPhysicalDeviceProperties2KHR(device, &props);

  std::fprintf(
    s_file,
    "Device %s Driver v%d.%d.%d API v%d.%d.%d (Device ID: 0x%x Vendor Id: "
    "0x%x)\n",
    props.properties.deviceName,
    VK_VERSION_MAJOR(props.properties.driverVersion),
    VK_VERSION_MINOR(props.properties.driverVersion),
    VK_VERSION_PATCH(props.properties.driverVersion),
    VK_VERSION_MAJOR(props.properties.apiVersion),
    VK_VERSION_MINOR(props.properties.apiVersion),
    VK_VERSION_PATCH(props.properties.apiVersion), props.properties.deviceID,
    props.properties.vendorID);

  std::fprintf(s_file, "    Type: %s\n",
               to_string(props.properties.deviceType).c_str());

  std::fprintf(s_file, "    Device UUID: ");
  for (int i = 0; i < VK_UUID_SIZE; ++i) {
    std::fprintf(s_file, "%x", id.deviceUUID[i]);
  }
  std::fprintf(s_file, "\n    Driver UUID: ");
  for (int i = 0; i < VK_UUID_SIZE; ++i) {
    std::fprintf(s_file, "%x", id.driverUUID[i]);
  }
  std::fprintf(s_file, "\n");

  std::fprintf(s_file,
               "    Multivew max instance count: %u max view count: %u\n",
               mp.maxMultiviewInstanceIndex, mp.maxMultiviewViewCount);

  VkPhysicalDevice16BitStorageFeaturesKHR d16;
  d16.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;
  d16.pNext = nullptr;

  VkPhysicalDeviceMultiviewFeaturesKHX mf;
  mf.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHX;
  mf.pNext = &d16;

  VkPhysicalDeviceFeatures2KHR features;
  features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
  features.pNext = &mf;

  getPhysicalDeviceFeatures2KHR(device, &features);

  std::fprintf(s_file, "    Features:\n");
  std::fprintf(s_file, "        robustBufferAccess: %s\n",
               (features.features.robustBufferAccess == VK_TRUE) ? "true"
                                                                 : "false");
  std::fprintf(s_file, "        fullDrawIndexUint32: %s\n",
               (features.features.fullDrawIndexUint32 == VK_TRUE) ? "true"
                                                                  : "false");
  std::fprintf(s_file, "        imageCubeArray: %s\n",
               (features.features.imageCubeArray == VK_TRUE) ? "true"
                                                             : "false");
  std::fprintf(s_file, "        independentBlend: %s\n",
               (features.features.independentBlend == VK_TRUE) ? "true"
                                                               : "false");
  std::fprintf(s_file, "        geometryShader: %s\n",
               (features.features.geometryShader == VK_TRUE) ? "true"
                                                             : "false");
  std::fprintf(s_file, "        tessellationShader: %s\n",
               (features.features.tessellationShader == VK_TRUE) ? "true"
                                                                 : "false");
  std::fprintf(s_file, "        sampleRateShading: %s\n",
               (features.features.sampleRateShading == VK_TRUE) ? "true"
                                                                : "false");
  std::fprintf(s_file, "        dualSrcBlend: %s\n",
               (features.features.dualSrcBlend == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "        logicOp: %s\n",
               (features.features.logicOp == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "        multiDrawIndirect: %s\n",
               (features.features.multiDrawIndirect == VK_TRUE) ? "true"
                                                                : "false");
  std::fprintf(s_file, "        drawIndirectFirstInstance: %s\n",
               (features.features.drawIndirectFirstInstance == VK_TRUE)
                 ? "true"
                 : "false");
  std::fprintf(s_file, "        depthClamp: %s\n",
               (features.features.depthClamp == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "        depthBiasClamp: %s\n",
               (features.features.depthBiasClamp == VK_TRUE) ? "true"
                                                             : "false");
  std::fprintf(s_file, "        fillModeNonSolid: %s\n",
               (features.features.fillModeNonSolid == VK_TRUE) ? "true"
                                                               : "false");
  std::fprintf(s_file, "        depthBounds: %s\n",
               (features.features.depthBounds == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "        wideLines: %s\n",
               (features.features.wideLines == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "        largePoints: %s\n",
               (features.features.largePoints == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "        alphaToOne: %s\n",
               (features.features.alphaToOne == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "        multiViewport: %s\n",
               (features.features.multiViewport == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "        samplerAnisotropy: %s\n",
               (features.features.samplerAnisotropy == VK_TRUE) ? "true"
                                                                : "false");
  std::fprintf(s_file, "        textureCompressionETC2: %s\n",
               (features.features.textureCompressionETC2 == VK_TRUE) ? "true"
                                                                     : "false");
  std::fprintf(s_file, "        textureCompressionASTC_LDR: %s\n",
               (features.features.textureCompressionASTC_LDR == VK_TRUE)
                 ? "true"
                 : "false");
  std::fprintf(s_file, "        textureCompressionBC: %s\n",
               (features.features.textureCompressionBC == VK_TRUE) ? "true"
                                                                   : "false");
  std::fprintf(s_file, "        occlusionQueryPrecise: %s\n",
               (features.features.occlusionQueryPrecise == VK_TRUE) ? "true"
                                                                    : "false");
  std::fprintf(
    s_file, "        pipelineStatisticsQuery: %s\n",
    (features.features.pipelineStatisticsQuery == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "        vertexPipelineStoresAndAtomics: %s\n",
               (features.features.vertexPipelineStoresAndAtomics == VK_TRUE)
                 ? "true"
                 : "false");
  std::fprintf(
    s_file, "        fragmentStoresAndAtomics: %s\n",
    (features.features.fragmentStoresAndAtomics == VK_TRUE) ? "true" : "false");
  std::fprintf(
    s_file, "        shaderTessellationAndGeometryPointSize: %s\n",
    (features.features.shaderTessellationAndGeometryPointSize == VK_TRUE)
      ? "true"
      : "false");
  std::fprintf(s_file, "        shaderImageGatherExtended: %s\n",
               (features.features.shaderImageGatherExtended == VK_TRUE)
                 ? "true"
                 : "false");
  std::fprintf(s_file, "        shaderStorageImageExtendedFormats: %s\n",
               (features.features.shaderStorageImageExtendedFormats == VK_TRUE)
                 ? "true"
                 : "false");
  std::fprintf(s_file, "        shaderStorageImageMultisample: %s\n",
               (features.features.shaderStorageImageMultisample == VK_TRUE)
                 ? "true"
                 : "false");
  std::fprintf(
    s_file, "        shaderStorageImageReadWithoutFormat: %s\n",
    (features.features.shaderStorageImageReadWithoutFormat == VK_TRUE)
      ? "true"
      : "false");
  std::fprintf(
    s_file, "        shaderStorageImageWriteWithoutFormat: %s\n",
    (features.features.shaderStorageImageWriteWithoutFormat == VK_TRUE)
      ? "true"
      : "false");
  std::fprintf(
    s_file, "        shaderUniformBufferArrayDynamicIndexing: %s\n",
    (features.features.shaderUniformBufferArrayDynamicIndexing == VK_TRUE)
      ? "true"
      : "false");
  std::fprintf(
    s_file, "        shaderSampledImageArrayDynamicIndexing: %s\n",
    (features.features.shaderSampledImageArrayDynamicIndexing == VK_TRUE)
      ? "true"
      : "false");
  std::fprintf(
    s_file, "        shaderStorageBufferArrayDynamicIndexing: %s\n",
    (features.features.shaderStorageBufferArrayDynamicIndexing == VK_TRUE)
      ? "true"
      : "false");
  std::fprintf(
    s_file, "        shaderStorageImageArrayDynamicIndexing: %s\n",
    (features.features.shaderStorageImageArrayDynamicIndexing == VK_TRUE)
      ? "true"
      : "false");
  std::fprintf(s_file, "        shaderClipDistance: %s\n",
               (features.features.shaderClipDistance == VK_TRUE) ? "true"
                                                                 : "false");
  std::fprintf(s_file, "        shaderCullDistance: %s\n",
               (features.features.shaderCullDistance == VK_TRUE) ? "true"
                                                                 : "false");
  std::fprintf(s_file, "        shaderFloat64: %s\n",
               (features.features.shaderFloat64 == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "        shaderInt64: %s\n",
               (features.features.shaderInt64 == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "        shaderInt16: %s\n",
               (features.features.shaderInt16 == VK_TRUE) ? "true" : "false");
  std::fprintf(
    s_file, "        shaderResourceResidency: %s\n",
    (features.features.shaderResourceResidency == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "        shaderResourceMinLod: %s\n",
               (features.features.shaderResourceMinLod == VK_TRUE) ? "true"
                                                                   : "false");
  std::fprintf(s_file, "        sparseBinding: %s\n",
               (features.features.sparseBinding == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "        sparseResidencyBuffer: %s\n",
               (features.features.sparseResidencyBuffer == VK_TRUE) ? "true"
                                                                    : "false");
  std::fprintf(s_file, "        sparseResidencyImage2D: %s\n",
               (features.features.sparseResidencyImage2D == VK_TRUE) ? "true"
                                                                     : "false");
  std::fprintf(s_file, "        sparseResidencyImage3D: %s\n",
               (features.features.sparseResidencyImage3D == VK_TRUE) ? "true"
                                                                     : "false");
  std::fprintf(
    s_file, "        sparseResidency2Samples: %s\n",
    (features.features.sparseResidency2Samples == VK_TRUE) ? "true" : "false");
  std::fprintf(
    s_file, "        sparseResidency4Samples: %s\n",
    (features.features.sparseResidency4Samples == VK_TRUE) ? "true" : "false");
  std::fprintf(
    s_file, "        sparseResidency8Samples: %s\n",
    (features.features.sparseResidency8Samples == VK_TRUE) ? "true" : "false");
  std::fprintf(
    s_file, "        sparseResidency16Samples: %s\n",
    (features.features.sparseResidency16Samples == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "        sparseResidencyAliased: %s\n",
               (features.features.sparseResidencyAliased == VK_TRUE) ? "true"
                                                                     : "false");
  std::fprintf(
    s_file, "        variableMultisampleRate: %s\n",
    (features.features.variableMultisampleRate == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "        inheritedQueries: %s\n",
               (features.features.inheritedQueries == VK_TRUE) ? "true"
                                                               : "false");

  std::fprintf(s_file, "        multiview: %s\n",
               (mf.multiview == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "            with geometry shaders: %s\n",
               (mf.multiviewGeometryShader == VK_TRUE) ? "true" : "false");
  std::fprintf(s_file, "            with tessellation shaders: %s\n",
               (mf.multiviewTessellationShader == VK_TRUE) ? "true" : "false");

  std::fprintf(
    s_file,
    "        16bit storage buffer: %s uniform: %s push constant: %s input "
    "output: %s\n",
    (d16.storageBuffer16BitAccess == VK_TRUE) ? "true" : "false",
    (d16.uniformAndStorageBuffer16BitAccess == VK_TRUE) ? "true" : "false",
    (d16.storagePushConstant16 == VK_TRUE) ? "true" : "false",
    (d16.storageInputOutput16 == VK_TRUE) ? "true" : "false");

  uint32_t num;
  VkResult rslt =
    vkEnumerateDeviceExtensionProperties(device, "", &num, nullptr);
  if (rslt != VK_SUCCESS) return vk::make_error_code(rslt);

  std::vector<VkExtensionProperties> exts(num);
  rslt = vkEnumerateDeviceExtensionProperties(device, "", &num, exts.data());
  if (rslt != VK_SUCCESS) return vk::make_error_code(rslt);

  std::fprintf(s_file, "    %u Device Extensions:\n", num);
  for (auto&& ext : exts) std::fprintf(s_file, "      %s\n", ext.extensionName);

  vkGetPhysicalDeviceQueueFamilyProperties(device, &num, nullptr);
  std::vector<VkQueueFamilyProperties2KHR> qfs(num);
  for (auto& qf : qfs) {
    qf.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2_KHR;
    qf.pNext = nullptr;
  }

  getPhysicalDeviceQueueFamilyProperties2KHR(device, &num, qfs.data());

  std::fprintf(s_file, "    %u Queue Families:\n", num);
  for (uint32_t i = 0; i < num; ++i) {
    auto const& qf = qfs[i].queueFamilyProperties;
    std::fprintf(
      s_file, "      Index %u Count %d Flags %s\n", i, qf.queueCount,
      to_string(static_cast<VkQueueFlagBits>(qf.queueFlags)).c_str());
  }

  VkPhysicalDeviceMemoryProperties2KHR mps;
  mps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2_KHR;
  mps.pNext = nullptr;

  getPhysicalDeviceMemoryProperties2KHR(device, &mps);

  std::fprintf(s_file, "    %u Memory Types:\n",
               mps.memoryProperties.memoryTypeCount);
  for (uint32_t i = 0; i < mps.memoryProperties.memoryTypeCount; ++i) {
    auto const& mem = mps.memoryProperties.memoryTypes[i];
    std::fprintf(
      s_file, "      Memory type %u properties %s heap %u\n", i,
      to_string(static_cast<VkMemoryPropertyFlagBits>(mem.propertyFlags))
        .c_str(),
      mem.heapIndex);
  }

  std::fprintf(s_file, "    %u Memory Heaps:\n",
               mps.memoryProperties.memoryHeapCount);
  for (uint32_t i = 0; i < mps.memoryProperties.memoryHeapCount; ++i) {
    auto const& heap = mps.memoryProperties.memoryHeaps[i];
    std::fprintf(
      s_file, "      Memory heap %u: %zu bytes (%g GiB) flags %s\n", i,
      heap.size, heap.size / 1024.f / 1024.f / 1024.f,
      to_string(static_cast<VkMemoryHeapFlagBits>(heap.flags)).c_str());
  }

  if (surface == VK_NULL_HANDLE) {
    return vk::make_error_code(vk::result::success);
  }

  VkPhysicalDeviceSurfaceInfo2KHR surface_info;
  surface_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
  surface_info.pNext = nullptr;
  surface_info.surface = surface;

  VkSurfaceCapabilities2KHR caps;
  caps.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
  caps.pNext = nullptr;

  rslt = getPhysicalDeviceSurfaceCapabilities2KHR(device, &surface_info, &caps);
  if (rslt != VK_SUCCESS) return vk::make_error_code(rslt);

  std::fprintf(s_file, "    Surface capabilities:\n");
  std::fprintf(
    s_file, "        min/max image count: %u %u max image array layers: %u\n",
    caps.surfaceCapabilities.minImageCount,
    caps.surfaceCapabilities.maxImageCount,
    caps.surfaceCapabilities.maxImageArrayLayers);
  std::fprintf(
    s_file,
    "        current extent: (%u, %u) min/max extent: (%u, %u) (%u, %u)\n",
    caps.surfaceCapabilities.currentExtent.width,
    caps.surfaceCapabilities.currentExtent.height,
    caps.surfaceCapabilities.minImageExtent.width,
    caps.surfaceCapabilities.minImageExtent.height,
    caps.surfaceCapabilities.maxImageExtent.width,
    caps.surfaceCapabilities.maxImageExtent.height);
  std::fprintf(s_file, "        current transform: %s\n",
               to_string(caps.surfaceCapabilities.currentTransform).c_str());
  std::fprintf(s_file, "        supported transforms: %s\n",
               to_string(static_cast<VkSurfaceTransformFlagBitsKHR>(
                           caps.surfaceCapabilities.supportedTransforms))
                 .c_str());
  std::fprintf(s_file, "        supported composite alpha: %s\n",
               to_string(static_cast<VkCompositeAlphaFlagBitsKHR>(
                           caps.surfaceCapabilities.supportedCompositeAlpha))
                 .c_str());
  std::fprintf(s_file, "        supported image usage: %s\n",
               to_string(static_cast<VkImageUsageFlagBits>(
                           caps.surfaceCapabilities.supportedUsageFlags))
                 .c_str());

  rslt =
    getPhysicalDeviceSurfaceFormats2KHR(device, &surface_info, &num, nullptr);
  if (rslt != VK_SUCCESS) return vk::make_error_code(rslt);

  std::vector<VkSurfaceFormat2KHR> fmts(num);
  for (auto&& fmt : fmts) {
    fmt.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
    fmt.pNext = nullptr;
  }

  rslt = getPhysicalDeviceSurfaceFormats2KHR(device, &surface_info, &num,
                                             fmts.data());
  if (rslt != VK_SUCCESS) return vk::make_error_code(rslt);

  std::fprintf(s_file, "    %u surface formats\n", num);
  for (auto&& fmt : fmts) {
    std::fprintf(s_file, "        format: %s colorSpace: %s\n",
                 to_string(fmt.surfaceFormat.format).c_str(),
                 to_string(fmt.surfaceFormat.colorSpace).c_str());
  }

  return vk::make_error_code(vk::result::success);
}

std::error_code dump_physical_devices(VkInstance instance,
                                      VkSurfaceKHR surface) {
  uint32_t num;
  VkResult rslt = vkEnumeratePhysicalDevices(instance, &num, nullptr);
  if (rslt != VK_SUCCESS) return vk::make_error_code(rslt);

  std::vector<VkPhysicalDevice> devs(num);
  rslt = vkEnumeratePhysicalDevices(instance, &num, devs.data());
  if (rslt != VK_SUCCESS) return vk::make_error_code(rslt);

  for (auto&& dev : devs) {
    auto const ec = dump_physical_device(instance, dev, surface);
    if (ec) {
      std::fprintf(stderr, "dumping device failed: %s", ec.message().c_str());
    }
  }

  return vk::make_error_code(vk::result::success);
}

#if TURF_TARGET_WIN32
int CALLBACK WinMain(::HINSTANCE, ::HINSTANCE, ::LPSTR, int) {
  fopen_s(&s_file, "vkinfo.txt", "wb");
#else
int main(void) {
  s_file = stdout;
#endif
  std::error_code ec;

  ec = dump_instance_layers();
  if (ec) {
    std::fprintf(stderr, "dumping instance layers failed: %s\n",
                 ec.message().c_str());
    std::exit(EXIT_FAILURE);
  }

  ec = dump_instance_extensions();
  if (ec) {
    std::fprintf(stderr, "dumping instance extensions failed: %s\n",
                 ec.message().c_str());
    std::exit(EXIT_FAILURE);
  }

  auto instance = create_instance(ec);
  if (ec) {
    std::fprintf(stderr, "instance creation failed: %s\n",
                 ec.message().c_str());
    std::exit(EXIT_FAILURE);
  }

  create_debug_report_callback(instance, ec);
  if (ec) {
    std::fprintf(stderr, "debug report callback creation failed: %s\n",
                 ec.message().c_str());
    std::exit(EXIT_FAILURE);
  }

#if 0
  auto window =
    wsi::window::create({{0, 0}, {900, 900}}, "vkinfo",
                        wsi::window_options::fullscreen_windowed, ec);
  if (ec) {
    std::fprintf(stderr, "window creation failed: %s", ec.message().c_str());
    std::exit(EXIT_FAILURE);
  }

  auto surface = create_surface(instance, window, ec);
  if (ec) {
    std::fprintf(stderr, "surface creation failed: %s\n", ec.message().c_str());
    std::exit(EXIT_FAILURE);
  }
#else
  VkSurfaceKHR surface = VK_NULL_HANDLE;
#endif

  ec = dump_physical_devices(instance, surface);
  if (ec) {
    std::fprintf(stderr, "enumerating physical devices failed: %s\n",
                 ec.message().c_str());
    std::exit(EXIT_FAILURE);
  }
}
