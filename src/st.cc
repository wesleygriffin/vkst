// Vulkan-based ShaderToy Example

#include <plat/core.h>
#include <plat/fs_notify.h>
#include <plat/log.h>
#include "renderer.h"
PLAT_PUSH_WARNING
PLAT_MSVC_DISABLE_WARNING(4201)
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
PLAT_POP_WARNING
#include <array>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#if TURF_TARGET_WIN32
#include <shellapi.h>
#endif

static bool s_igpu{false}; // force integrated gpu
static renderer s_renderer;
static wsi::window s_window;
static surface s_surface;

// s_command_buffers has one recorded command buffer
// for each image in the surface swapchain
static std::vector<VkCommandBuffer> s_command_buffers;

static std::array<VkClearValue, 3> s_clear_values;

static shader s_vshader, s_fshader;
static VkPipelineLayout s_layout;
static VkPipeline s_pipeline;
static bool s_resize{false};
static bool s_rebuild{false};

// s_update_push_constants_{command_buffers, fences} have one command buffer
// or fence for each image in the surface swapchain to update push constants
static std::vector<VkCommandBuffer> s_update_push_constants_command_buffers;
static std::vector<VkFence> s_update_push_constants_fences;

// These are the shader uniforms.
// Currently the uniforms update at different rates:
//   - iMouse only on input
//   - iTime, iTimeDelta, iFrameRate, iFrame each frame
//   - iResolution only on resize
// So they should probably be broken out into different sets.
struct push_constant_uniform_block {
  glm::vec4 iMouse{0.f, 0.f, 0.f, 0.f};
  float iTime{0.f};
  float iTimeDelta{0.f};
  float iFrameRate{0.f};
  float iFrame{0};
  glm::vec3 iResolution{0.f, 0.f, 0.f};
  float padding0;
} s_shader_push_constants;

// Create a full pipeline with a full-screen quad vertex shader
static std::tuple<shader, shader, VkPipelineLayout, VkPipeline>
create_pipeline(std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  shader vshader{};
  shader fshader{};
  VkPipelineLayout layout{VK_NULL_HANDLE};
  std::vector<VkPipeline> pipelines{};

  vshader = s_renderer.create_shader(
    PROJECT_DIR "/assets/shaders/fsq.vert", shader::types::vertex, ec);
  if (ec) {
    LOG_FATAL(
      "creating shader " PROJECT_DIR "/assets/shaders/fsq.vert failed: %s%s%s",
      ec.message().c_str(), (vshader.error_message().empty() ? "" : "\n"),
      vshader.error_message().c_str());
    return std::make_tuple(std::move(vshader), std::move(fshader), layout,
                           VkPipeline{VK_NULL_HANDLE});
  }

  fshader = s_renderer.create_shader(
    PROJECT_DIR "/assets/shaders/shadertoy.frag", shader::types::fragment, ec);
  if (ec) {
    LOG_FATAL("creating shader " PROJECT_DIR
              "/assets/shaders/shadertoy.frag failed: %s%s%s",
              ec.message().c_str(), (fshader.error_message().empty() ? "" : "\n"),
              fshader.error_message().c_str());
    return std::make_tuple(std::move(vshader), std::move(fshader), layout,
                           VkPipeline{VK_NULL_HANDLE});
  }

  std::array<VkPipelineShaderStageCreateInfo, 2> stages{{
    {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
     VK_SHADER_STAGE_VERTEX_BIT, vshader, "main", nullptr},
    {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
     VK_SHADER_STAGE_FRAGMENT_BIT, fshader, "main", nullptr},
  }};


  // There are no binding or attribute descriptions for the vertex input as
  // the fsq.vert vertex shader just uses gl_VertexIndex to create a triangle
  VkPipelineVertexInputStateCreateInfo vertex_input = {};
  vertex_input.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
  input_assembly.sType =
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  // The viewport and scissor are specified later as dynamic states
  VkPipelineViewportStateCreateInfo viewport = {};
  viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport.viewportCount = 1;
  viewport.pViewports = nullptr;
  viewport.scissorCount = 1;
  viewport.pScissors = nullptr;

  // fsq.vert outputs the vertices in clock-wise order
  VkPipelineRasterizationStateCreateInfo rasterization = {};
  rasterization.sType =
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterization.polygonMode = VK_POLYGON_MODE_FILL;
  rasterization.cullMode = VK_CULL_MODE_FRONT_BIT;
  rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterization.lineWidth = 1.f;

  VkPipelineMultisampleStateCreateInfo multisample = {};
  multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample.rasterizationSamples = s_surface.samples();
  multisample.minSampleShading = 1.f;

  VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
  depth_stencil.sType =
    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_stencil.depthWriteEnable = VK_TRUE;

  VkPipelineColorBlendAttachmentState color_blend_attachment = {};
  color_blend_attachment.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo color_blend = {};
  color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blend.attachmentCount = 1;
  color_blend.pAttachments = &color_blend_attachment;

  std::array<VkDynamicState, 2> dynamic_states = {
    {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR}};

  VkPipelineDynamicStateCreateInfo dynamic = {};
  dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic.dynamicStateCount = gsl::narrow_cast<uint32_t>(dynamic_states.size());
  dynamic.pDynamicStates = dynamic_states.data();

  VkPushConstantRange push_constant_range = {};
  push_constant_range.stageFlags =
    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  push_constant_range.offset = 0;
  push_constant_range.size = sizeof(s_shader_push_constants);

  layout = s_renderer.create_pipeline_layout({}, push_constant_range, ec);
  if (ec) {
    return std::make_tuple(std::move(vshader), std::move(fshader), layout,
                           VkPipeline{VK_NULL_HANDLE});
  }

  VkGraphicsPipelineCreateInfo cinfo = {};
  cinfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  cinfo.stageCount = gsl::narrow_cast<uint32_t>(stages.size());
  cinfo.pStages = stages.data();
  cinfo.pVertexInputState = &vertex_input;
  cinfo.pInputAssemblyState = &input_assembly;
  cinfo.pViewportState = &viewport;
  cinfo.pRasterizationState = &rasterization;
  cinfo.pMultisampleState = &multisample;
  cinfo.pDepthStencilState = &depth_stencil;
  cinfo.pColorBlendState = &color_blend;
  cinfo.pDynamicState = &dynamic;
  cinfo.layout = layout;
  cinfo.renderPass = s_surface.render_pass();
  cinfo.subpass = 0;

  pipelines = s_renderer.create_pipelines(cinfo, ec);
  if (ec) {
    return std::make_tuple(std::move(vshader), std::move(fshader), layout,
                           VkPipeline{VK_NULL_HANDLE});
  }

  LOG_LEAVE;
  return std::make_tuple(std::move(vshader), std::move(fshader), layout,
                         pipelines[0]);
} // create_pipeline

// Log a Vulkan debug report callback message
static VkBool32 debug_report(VkDebugReportFlagsEXT flags,
                             VkDebugReportObjectTypeEXT, uint64_t, size_t,
                             int32_t, char const* layer_prefix,
                             char const* message, void*) noexcept {
  if ((flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) ==
      VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
    LOG_INFO("%s: %s", layer_prefix, message);
  }

  if ((flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) ==
      VK_DEBUG_REPORT_WARNING_BIT_EXT) {
    LOG_WARN("%s: %s", layer_prefix, message);
  }

  if ((flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) ==
      VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
    LOG_WARN("%s: %s", layer_prefix, message);
  }

  if ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) ==
      VK_DEBUG_REPORT_ERROR_BIT_EXT) {
    LOG_ERROR("%s: %s", layer_prefix, message);
  }

  if ((flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) ==
      VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
    if (strcmp(layer_prefix, "loader") != 0) {
      LOG_DEBUG("%s: %s", layer_prefix, message);
    }
  }

  return VK_FALSE;
}

// Pre-record the command buffers to bind the pipeline and draw the vertices
static void record_command_buffers(gsl::span<VkCommandBuffer> command_buffers,
                                   VkPipeline pipeline) noexcept {
  LOG_ENTER;

  static VkCommandBufferBeginInfo cbinfo = {};
  cbinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cbinfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

  // Use the renderpass from the surface
  static VkRenderPassBeginInfo rbinfo = {};
  rbinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rbinfo.renderPass = s_surface.render_pass();
  rbinfo.renderArea.extent = {
    gsl::narrow_cast<uint32_t>(s_window.size().width),
    gsl::narrow_cast<uint32_t>(s_window.size().height)};
  rbinfo.clearValueCount = gsl::narrow_cast<uint32_t>(s_clear_values.size());
  rbinfo.pClearValues = s_clear_values.data();

  for (std::size_t i = 0; i < command_buffers.size(); ++i) {
    rbinfo.framebuffer = s_surface.framebuffer(i);

    vkBeginCommandBuffer(command_buffers[i], &cbinfo);

    vkCmdSetViewport(command_buffers[i], 0, 1, &s_surface.viewport());
    vkCmdSetScissor(command_buffers[i], 0, 1, &s_surface.scissor());

    vkCmdBeginRenderPass(command_buffers[i], &rbinfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline);
    vkCmdDraw(command_buffers[i], 3, 1, 0, 0);

    vkCmdEndRenderPass(command_buffers[i]);
    vkEndCommandBuffer(command_buffers[i]);
  }

  LOG_LEAVE;
}

static void init(std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  s_renderer = renderer::create(
    "st",
    (s_igpu ? renderer_options::use_integrated_gpu : renderer_options::none),
    &debug_report, sizeof(s_shader_push_constants), ec);
  if (ec) return;

  s_window = wsi::window::create(
    {{0, 0}, {900, 900}}, "st",
    wsi::window_options::sizeable | wsi::window_options::decorated, ec);
  if (ec) return;

  s_surface = s_renderer.create_surface(s_window, ec);
  if (ec) return;

  s_command_buffers = s_renderer.allocate_command_buffers(
    gsl::narrow_cast<uint32_t>(s_surface.num_images()), ec);
  if (ec) return;

  s_update_push_constants_command_buffers = s_renderer.allocate_command_buffers(
    gsl::narrow_cast<uint32_t>(s_surface.num_images()), ec);
  if (ec) return;

  s_update_push_constants_fences.reserve(s_surface.num_images());
  for (std::size_t i = 0; i < s_surface.num_images(); ++i) {
    s_update_push_constants_fences.push_back(s_renderer.create_fence(true, ec));
    if (ec) return;
  }

  s_clear_values[0] = {0.2f, 0.f, 0.3f, 0.f};
  s_clear_values[2] = {1.f, 0};

  std::tie(s_vshader, s_fshader, s_layout, s_pipeline) = create_pipeline(ec);
  if (ec) return;

  record_command_buffers(s_command_buffers, s_pipeline);

  LOG_LEAVE;
} // init

// Record a command buffer to update the shader push constants
// Use one comman buffer/fence for each surface swapchain image
static void update_push_constants(uint32_t index,
                                  std::error_code& ec) noexcept {
  // Wait on the fence for this swapchain image, after the wait returns
  // the associated command buffer is free to be recorded into
  s_renderer.wait(s_update_push_constants_fences[index], true, UINT64_MAX, ec);
  if (ec) return;
  s_renderer.reset(s_update_push_constants_fences[index], ec);
  if (ec) return;

  static VkCommandBufferBeginInfo cbinfo = {};
  cbinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cbinfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

  vkBeginCommandBuffer(s_update_push_constants_command_buffers[index], &cbinfo);

  vkCmdPushConstants(s_update_push_constants_command_buffers[index], s_layout,
                     VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                     0, sizeof(s_shader_push_constants),
                     &s_shader_push_constants);

  vkEndCommandBuffer(s_update_push_constants_command_buffers[index]);
} // update_push_constants

static void draw() {
  std::error_code ec;

  uint32_t image_index = s_renderer.acquire_next_image(s_surface, ec);
  if (ec) {
    if (ec.value() == VK_SUBOPTIMAL_KHR ||
        ec.value() == VK_ERROR_OUT_OF_DATE_KHR) {
      s_resize = true;
    } else {
      LOG_FATAL("draw: acquire next image failed: %s", ec.message().c_str());
      s_window.close();
      return;
    }
  }

  update_push_constants(image_index, ec);
  if (ec) {
    LOG_FATAL("update push constants failed: %s", ec.message().c_str());
    s_window.close();
    return;
  }

  std::array<VkCommandBuffer, 2> submit_command_buffers = {{
    s_update_push_constants_command_buffers[image_index],
    s_command_buffers[image_index],
  }};

  // Both submit the command buffers and then present the swapchain image
  s_renderer.submit_present(submit_command_buffers, s_surface, image_index,
                            s_update_push_constants_fences[image_index], ec);
  if (ec) {
    if (ec.value() == VK_SUBOPTIMAL_KHR ||
        ec.value() == VK_ERROR_OUT_OF_DATE_KHR) {
      s_resize = true;
    } else {
      LOG_FATAL("draw: submit and present failed: %s", ec.message().c_str());
      s_window.close();
      return;
    }
  }
} // draw

static void resize() {
  LOG_ENTER;
  std::error_code ec;

  auto const size = s_window.size();
  s_renderer.resize(s_surface, size, ec);
  if (ec) {
    LOG_FATAL("resize: surface resize failed: %s", ec.message().c_str());
    s_window.close();
    return;
  }

  s_shader_push_constants.iResolution.x = static_cast<float>(size.width);
  s_shader_push_constants.iResolution.y = static_cast<float>(size.height);
  s_shader_push_constants.iResolution.z =
    s_shader_push_constants.iResolution.x /
    s_shader_push_constants.iResolution.y;

  // The surface swapchain has changed, so re-record the command buffers
  record_command_buffers(s_command_buffers, s_pipeline);
  s_resize = false;

  LOG_LEAVE;
} // resize

// The shaders have changed, so rebuild the command buffers and pipeline
static void rebuild() {
  LOG_ENTER;
  std::error_code ec;

  shader new_vshader, new_fshader;
  VkPipelineLayout new_layout{VK_NULL_HANDLE};
  VkPipeline new_pipeline{VK_NULL_HANDLE};
  std::vector<VkCommandBuffer> new_command_buffers;

  std::tie(new_vshader, new_fshader, new_layout, new_pipeline) =
    create_pipeline(ec);
  if (ec) {
    LOG_FATAL("rebuild: creating pipeline failed: %s", ec.message().c_str());
    goto fail;
  }

  new_command_buffers = s_renderer.allocate_command_buffers(
    gsl::narrow_cast<uint32_t>(s_surface.num_images()), ec);
  if (ec) {
    LOG_FATAL("rebuild: allocating command buffers failed: %s",
              ec.message().c_str());
    goto fail;
  }

  record_command_buffers(new_command_buffers, new_pipeline);

  s_renderer.free(s_command_buffers);
  s_renderer.destroy(s_pipeline);
  s_renderer.destroy(s_layout);
  s_renderer.destroy(s_fshader);
  s_renderer.destroy(s_vshader);

  s_command_buffers = std::move(new_command_buffers);
  s_pipeline = new_pipeline;
  s_layout = new_layout;
  s_fshader = std::move(new_fshader);
  s_vshader = std::move(new_vshader);

  s_rebuild = false;
  LOG_LEAVE;
  return;

fail:
  s_renderer.free(new_command_buffers);
  s_renderer.destroy(new_pipeline);
  s_renderer.destroy(new_layout);
  s_renderer.destroy(new_fshader);
  s_renderer.destroy(new_vshader);
} // rebuild

#if TURF_TARGET_WIN32

void parse_options(LPWSTR* szArgList, int nArgs) {
  for (int i = 0; i < nArgs; ++i) {
    if (wcscmp(szArgList[i], L"--igpu") == 0) s_igpu = true;
  }
} // parse_options

#else

void parse_options(int argc, char* argv[]) {
  for (int i = 0; i < argc; ++i) {
    if (strcmp(argv[i], "--igpu") == 0) s_igpu = true;
  }
} // parse_options

#endif // TURF_TARGET_WIN32

#if TURF_TARGET_WIN32
int CALLBACK WinMain(::HINSTANCE, ::HINSTANCE, ::LPSTR, int) {
#else
int main(int argc, char* argv[]) {
#endif
  std::error_code ec;

  plat::init_logging("st.log", ec);
  if (ec) std::exit(EXIT_FAILURE);

#if TURF_TARGET_WIN32
  int nArgs;
  LPWSTR* szArgList = ::CommandLineToArgvW(GetCommandLineW(), &nArgs);
  parse_options(szArgList, nArgs);
  LocalFree(szArgList);
#else
  parse_options(argc, argv);
#endif

  init(ec);
  if (ec) {
    LOG_FATAL("initialization failed: %s", ec.message().c_str());
    std::exit(EXIT_FAILURE);
  }

  s_window.on_resize([](auto, auto) { s_resize = true; });

  auto input = wsi::input{&s_window};
  s_window.show();
  resize();

  auto shader_changed = [](auto, auto path, auto action) {
    if (action != plat::fs_notify::actions::removed) return;
    s_rebuild = true;
  };

  plat::fs_notify watcher;

  watcher.add(PROJECT_DIR "/assets/shaders/fsq.vert", shader_changed, false,
              ec);
  if (ec) {
    LOG_ERROR("watching " PROJECT_DIR "/assets/shaders/fsq.vert failed: %s",
              ec.message().c_str());
  }

  watcher.add(PROJECT_DIR "/assets/shaders/shadertoy.frag", shader_changed,
              false, ec);
  if (ec) {
    LOG_ERROR("watching " PROJECT_DIR
              "/assets/shaders/shadertoy.frag failed: %s",
              ec.message().c_str());
  }

  auto start{std::chrono::steady_clock::now()}, last{start};
  int32_t frame{0};

  LOG_TRACE("running");
  while (!s_window.closed()) {
    auto const now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed{now - start}, delta{now - last};
    last = now;

    s_window.poll_events();
    if (s_resize) resize();
    watcher.tick();
    if (s_rebuild) rebuild();
    input.tick();

    if (input.key_released(wsi::keys::eEscape)) break;

    if (input.button_down(wsi::buttons::e1)) {
      s_shader_push_constants.iMouse.x =
        static_cast<float>(s_window.cursor_pos().x);
      s_shader_push_constants.iMouse.y =
        static_cast<float>(s_window.cursor_pos().y);
    } else if (input.button_released(wsi::buttons::e1)) {
      s_shader_push_constants.iMouse.z =
        static_cast<float>(s_window.cursor_pos().x);
      s_shader_push_constants.iMouse.w =
        static_cast<float>(s_window.cursor_pos().y);
    }

    s_shader_push_constants.iTime = elapsed.count();
    s_shader_push_constants.iTimeDelta = delta.count();
    s_shader_push_constants.iFrame = static_cast<float>(frame);
    s_shader_push_constants.iFrameRate = frame / s_shader_push_constants.iTime;

    draw();
    frame += 1;
  }
  LOG_TRACE("done");

  s_renderer.free(s_update_push_constants_command_buffers);
  for (auto&& fence : s_update_push_constants_fences) s_renderer.destroy(fence);

  s_renderer.free(s_command_buffers);
  s_renderer.destroy(s_pipeline);
  s_renderer.destroy(s_layout);
  s_renderer.destroy(s_fshader);
  s_renderer.destroy(s_vshader);
  s_renderer.destroy(s_surface);
  return 0;
}
