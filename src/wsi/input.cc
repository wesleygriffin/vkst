#include "input.h"
#include "window.h"

wsi::offset2d wsi::input::cursor_pos() const noexcept {
  return _win->cursor_pos();
} // wsi::input::cursor_pos()

wsi::input::input(gsl::not_null<window*> win) noexcept
: _win{win}
, _prev_keys{}
, _curr_keys{}
, _prev_buttons{}
, _curr_buttons{}
, _prev_scroll{0}
, _curr_scroll{0}
, _prev_pos{}
, _curr_pos{} {} // wsi::input::input(

void wsi::input::tick() noexcept {
  _prev_keys = std::move(_curr_keys);
  _curr_keys = _win->keys();
  _prev_buttons = std::move(_curr_buttons);
  _curr_buttons = _win->buttons();
  _prev_scroll = _curr_scroll;
  _curr_scroll = _win->scroll();
  _prev_pos = std::move(_curr_pos);
  _curr_pos = _win->cursor_pos();
} // wsi::input::tick()
