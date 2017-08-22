#include <plat/fs_notify.h>
#include <plat/log.h>
#include "renderer.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <array>
#include <chrono>
#include <cstdlib>
#include <cstdio>

static renderer s_renderer;
static wsi::window s_window;
static surface s_surface;
static std::vector<VkCommandBuffer> s_command_buffers;
static std::array<VkClearValue, 3> s_clear_values;
static VkShaderModule s_vshader, s_fshader;
static VkPipelineLayout s_layout;
static std::vector<VkPipeline> s_pipelines;
static bool s_resize{false};
static bool s_rebuild{false};

static std::vector<VkCommandBuffer> s_update_push_constants_command_buffers;
static std::vector<VkFence> s_update_push_constants_fences;

struct push_constant_uniform_block {
  glm::vec4 iMouse{0.f, 0.f, 0.f, 0.f};
  float iTime{0.f};
  float iTimeDelta{0.f};
  float iFrameRate{0.f};
  int iFrame{0};
  glm::vec3 iResolution{0.f, 0.f, 0.f};
} s_shader_push_constants;

static void create_pipeline(std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  s_vshader = s_renderer.create_shader(PROJECT_DIR "/assets/shaders/fsq.vert",
                                       renderer::shader_types::vertex, ec);
  if (ec) return;

  s_fshader =
    s_renderer.create_shader(PROJECT_DIR "/assets/shaders/shadertoy.frag",
                             renderer::shader_types::fragment, ec);
  if (ec) return;

  std::array<VkPipelineShaderStageCreateInfo, 2> stages{{
    {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
     VK_SHADER_STAGE_VERTEX_BIT, s_vshader, "main", nullptr},
    {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
     VK_SHADER_STAGE_FRAGMENT_BIT, s_fshader, "main", nullptr},
  }};


  VkPipelineVertexInputStateCreateInfo vertex_input = {};
  vertex_input.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
  input_assembly.sType =
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkPipelineViewportStateCreateInfo viewport = {};
  viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport.viewportCount = 1;
  viewport.pViewports = nullptr;
  viewport.scissorCount = 1;
  viewport.pScissors = nullptr;

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

  s_layout = s_renderer.create_pipeline_layout(
    {}, gsl::make_span(&push_constant_range, 1), ec);
  if (ec) return;

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
  cinfo.layout = s_layout;
  cinfo.renderPass = s_surface.render_pass();
  cinfo.subpass = 0;

  s_pipelines = s_renderer.create_pipelines(gsl::make_span(&cinfo, 1), ec);
  if (ec) return;

  LOG_LEAVE;
} // create_pipeline

static void init(std::error_code& ec) noexcept {
  LOG_ENTER;
  ec.clear();

  s_renderer = renderer::create("st", ec);
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
  create_pipeline(ec);
  if (ec) return;

  LOG_LEAVE;
} // init

static void update_push_constants(uint32_t index,
                                  std::error_code& ec) noexcept {
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

static void record_command_buffers() {
  LOG_ENTER;

  static VkCommandBufferBeginInfo cbinfo = {};
  cbinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cbinfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

  static VkRenderPassBeginInfo rbinfo = {};
  rbinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rbinfo.renderPass = s_surface.render_pass();
  rbinfo.renderArea.extent = {
    gsl::narrow_cast<uint32_t>(s_window.size().width),
    gsl::narrow_cast<uint32_t>(s_window.size().height)};
  rbinfo.clearValueCount = gsl::narrow_cast<uint32_t>(s_clear_values.size());
  rbinfo.pClearValues = s_clear_values.data();

  for (std::size_t i = 0; i < s_command_buffers.size(); ++i) {
    rbinfo.framebuffer = s_surface.framebuffer(i);

    vkBeginCommandBuffer(s_command_buffers[i], &cbinfo);

    vkCmdSetViewport(s_command_buffers[i], 0, 1, &s_surface.viewport());
    vkCmdSetScissor(s_command_buffers[i], 0, 1, &s_surface.scissor());

    vkCmdBeginRenderPass(s_command_buffers[i], &rbinfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(s_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                      s_pipelines[0]);
    vkCmdDraw(s_command_buffers[i], 3, 1, 0, 0);

    vkCmdEndRenderPass(s_command_buffers[i]);
    vkEndCommandBuffer(s_command_buffers[i]);
  }

  LOG_LEAVE;
}

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

  record_command_buffers();
  s_resize = false;

  LOG_LEAVE;
} // resize

static void rebuild() {
  LOG_ENTER;
  std::error_code ec;

  s_renderer.free(s_command_buffers);
  s_renderer.destroy(s_pipelines);
  s_renderer.destroy(s_layout);
  s_renderer.destroy(s_fshader);
  s_renderer.destroy(s_vshader);

  s_command_buffers = s_renderer.allocate_command_buffers(
    gsl::narrow_cast<uint32_t>(s_surface.num_images()), ec);
  if (ec) {
    LOG_FATAL("rebuild: allocating command buffers failed: %s",
              ec.message().c_str());
    s_window.close();
    return;
  }

  create_pipeline(ec);
  if (ec) {
    LOG_FATAL("rebuild: creating pipeline failed: %s", ec.message().c_str());
    s_window.close();
    return;
  }

  record_command_buffers();
  s_rebuild = false;
  LOG_LEAVE;
}

int CALLBACK WinMain(::HINSTANCE, ::HINSTANCE, ::LPSTR, int) {
  std::error_code ec;

  plat::init_logging("st.log", ec);
  if (ec) std::exit(EXIT_FAILURE);

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
    s_shader_push_constants.iFrame = frame;
    s_shader_push_constants.iFrameRate = frame / s_shader_push_constants.iTime;

    draw();
    frame += 1;
  }
  LOG_TRACE("done");

  s_renderer.free(s_update_push_constants_command_buffers);
  for (auto&& fence : s_update_push_constants_fences) s_renderer.destroy(fence);

  s_renderer.free(s_command_buffers);
  s_renderer.destroy(s_pipelines);
  s_renderer.destroy(s_layout);
  s_renderer.destroy(s_fshader);
  s_renderer.destroy(s_vshader);
  s_renderer.destroy(s_surface);
  return 0;
}
