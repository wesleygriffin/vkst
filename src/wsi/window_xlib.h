#ifndef VKST_WSI_WINDOW_XLIB_H
#define VKST_WSI_WINDOW_XLIB_H

#ifndef VKST_WSI_WINDOW_H
#error "Include window.h only"
#endif

#include "window.h"
#include <X11/Xlib.h>
#include <array>

namespace wsi {

class window final : public impl::window<window> {
public:
  static window create(rect2d topleft_size, gsl::czstring title,
                       window_options opts, int screen_number,
                       std::error_code& ec) noexcept;

  constexpr window() noexcept {}
  window(window const&) = delete;
  window(window&& other) noexcept;
  window& operator=(window const&) = delete;
  window& operator=(window&& rhs) noexcept;
  ~window() noexcept;

  using native_handle_t = std::tuple<Display*, Visual*, Window>;
  native_handle_t native_handle() const noexcept {
    return std::make_tuple(_display, _visual, _handle);
  }

private:
  constexpr window(rect2d topleft_size) noexcept
  : impl::window<window>{std::move(topleft_size)} {}

  enum Atoms {
    WM_PROTOCOLS,
    WM_DELETE_WINDOW,
    NET_WM_NAME,
    NET_WM_ICON_NAME,
    _MOTIF_WM_HINTS,
    NUM_ATOMS,
  }; // enum Atoms

  Display* _display{nullptr};
  Visual* _visual{nullptr};
  Window _handle{0};
  std::array<wsi::keys, 256> _key_lut{};
  std::array<Atom, NUM_ATOMS> _atoms{};

  void do_retitle(gsl::czstring title) noexcept;
  std::string do_title() const noexcept;

  void do_resize(extent2d const& size) noexcept;
  void do_reposition(offset2d const& position) noexcept;

  void do_show() noexcept;
  void do_hide() noexcept;

  void do_close() noexcept;

  offset2d do_cursor_pos() const noexcept;

  void do_poll_events() noexcept;

  static gsl::czstring atom_to_string(Atoms atom) noexcept;

  friend class impl::window<window>;
}; // class window

} // namespace wsi

#endif // VKST_WSI_WINDOW_XLIB_H
