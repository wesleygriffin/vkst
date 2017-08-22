#ifndef VKST_WSI_WINDOW_WIN32_H
#define VKST_WSI_WINDOW_WIN32_H

#ifndef VKST_WSI_WINDOW_H
#error "Include window.h only"
#endif

#include "window.h"
#include <array>

namespace wsi {

class window final : public impl::window<window> {
public:
  static window create(rect2d topleft_size, gsl::czstring title,
                       window_options opts, std::error_code& ec) noexcept;

  constexpr window() noexcept {}
  window(window const&) = delete;
  window(window&& other) noexcept;
  window& operator=(window const&) = delete;
  window& operator=(window&& rhs) noexcept;
  ~window() noexcept;

  using native_handle_t = std::pair<HINSTANCE, HWND>;
  native_handle_t native_handle() const noexcept {
    return std::make_pair(_instance, _handle);
  }

private:
  constexpr window(rect2d topleft_size) noexcept
  : impl::window<window>{std::move(topleft_size)} {}

  HINSTANCE _instance{0};
  HWND _handle{0};
  std::array<wsi::keys, 256> _key_lut{};

  void do_retitle(gsl::czstring title) noexcept;
  std::string do_title() const noexcept;

  void do_resize(extent2d const& size) noexcept;
  void do_reposition(offset2d const& size) noexcept;

  void do_show() noexcept;
  void do_hide() noexcept;

  void do_close() noexcept;

  offset2d do_cursor_pos() const noexcept;

  void do_poll_events() noexcept;

  LRESULT wnd_proc(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                  LPARAM lParam) noexcept;

  friend class impl::window<window>;
}; // class window

} // namespace wsi

#endif // VKST_WSI_WINDOW_WIN32_H