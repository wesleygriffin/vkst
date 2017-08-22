#ifndef VKST_WSI_WINDOW_H
#define VKST_WSI_WINDOW_H

#include <turf/c/core.h>
#include <gsl.h>
#include <wsi/input.h>
#include <functional>

namespace wsi {

enum class window_options : uint8_t {
  none = 0,
  decorated = (1 << 1),
  sizeable = (1 << 2),
  fullscreen_windowed = (1 << 3),
}; // enum class window_options

namespace impl {

template <class D>
class window {
public:
  void retitle(gsl::czstring title) noexcept {
    static_cast<D*>(this)->do_retitle(title);
  }
  std::string title() const noexcept {
    return static_cast<D const*>(this)->do_title();
  }

  rect2d const& topleft_size() const noexcept { return _topleft_size; }

  extent2d const& size() const noexcept { return _topleft_size.extent; }
  void resize(extent2d const& size) noexcept {
    static_cast<D*>(this)->do_resize(size);
  }

  offset2d const& position() const noexcept { return _topleft_size.offset; }
  void reposition(offset2d const& position) noexcept {
    static_cast<D*>(this)->do_reposition(position);
  }

  void show() noexcept { static_cast<D*>(this)->do_show(); }
  void hide() noexcept { static_cast<D*>(this)->do_hide(); }

  void close() noexcept { static_cast<D*>(this)->do_close(); }
  bool closed() const noexcept { return _closed; }

  keyset const& keys() const noexcept { return _keys; }
  buttonset const& buttons() const noexcept { return _buttons; }
  int scroll() const noexcept { return _scroll; }

  offset2d cursor_pos() const noexcept {
    return static_cast<D const*>(this)->do_cursor_pos();
  }

  void poll_events() noexcept { static_cast<D*>(this)->do_poll_events(); }

  using resize_delegate = std::function<void(D*, extent2d const&)>;
  void on_resize(resize_delegate delegate) noexcept {
    _on_resize = std::move(delegate);
  }

  using reposition_delegate = std::function<void(D*, offset2d const&)>;
  void on_reposition(reposition_delegate delegate) noexcept {
    _on_reposition = std::move(delegate);
  }

  using close_delegate = std::function<void(D*)>;
  void on_close(close_delegate delegate) noexcept {
    _on_close = std::move(delegate);
  }

  constexpr window() noexcept {}
  window(rect2d topleft_size) noexcept
  : _topleft_size{std::move(topleft_size)} {}

protected:
  rect2d _topleft_size{};
  bool _closed{false};
  keyset _keys{};
  buttonset _buttons{};
  int _scroll{0};

  resize_delegate _on_resize{[](auto, auto) {}};
  reposition_delegate _on_reposition{[](auto, auto) {}};
  close_delegate _on_close{[](D*) {}};
}; // class window

} // namespace impl

inline constexpr auto operator|(window_options a, window_options b) noexcept {
  using U = std::underlying_type_t<window_options>;
  return static_cast<window_options>(static_cast<U>(a) | static_cast<U>(b));
}

inline constexpr auto operator&(window_options a, window_options b) noexcept {
  using U = std::underlying_type_t<window_options>;
  return static_cast<window_options>(static_cast<U>(a) & static_cast<U>(b));
}

} // namespace wsi

// clang-format off
#if TURF_TARGET_WIN32
#  include <wsi/window_win32.h>
#elif TURF_KERNEL_LINUX
#  include <wsi/window_xlib.h>
#else
#  error "Unsupported platform"
#endif
// clang-format on

#endif // VKST_WSI_WINDOW_H
