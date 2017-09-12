#include "window.h"
#include <plat/core.h>

#if TURF_KERNEL_LINUX
#include <cstring>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

namespace {

wsi::keys translate_keysym(KeySym keysym) {
  using namespace wsi;
  switch (keysym) {
  case XK_BackSpace: return keys::eBackspace;
  case XK_Tab: return keys::eTab;
  case XK_Return: return keys::eEnter;
  case XK_Pause: return keys::ePause;
  case XK_Scroll_Lock: return keys::eScrollLock;
  case XK_Escape: return keys::eEscape;
  case XK_Delete: return keys::eDelete;

  case XK_Home: return keys::eHome;
  case XK_Left: return keys::eLeft;
  case XK_Up: return keys::eUp;
  case XK_Down: return keys::eDown;
  case XK_Page_Up: return keys::ePageUp;
  case XK_Page_Down: return keys::ePageDown;
  case XK_End: return keys::eEnd;

  case XK_Insert: return keys::eInsert;
  case XK_Num_Lock: return keys::eNumLock;

  case XK_KP_Enter: return keys::eKeypadEnter;
  case XK_KP_Home: return keys::eKeypad7;
  case XK_KP_Left: return keys::eKeypad4;
  case XK_KP_Up: return keys::eKeypad8;
  case XK_KP_Right: return keys::eKeypad6;
  case XK_KP_Down: return keys::eKeypad2;
  case XK_KP_Page_Up: return keys::eKeypad9;
  case XK_KP_Page_Down: return keys::eKeypad3;
  case XK_KP_End: return keys::eKeypad1;
  case XK_KP_Insert: return keys::eKeypad0;
  case XK_KP_Delete: return keys::eKeypadDecimal;
  case XK_KP_Multiply: return keys::eKeypadMultiply;
  case XK_KP_Add: return keys::eKeypadAdd;
  case XK_KP_Subtract: return keys::eKeypadSubtract;
  case XK_KP_Divide: return keys::eKeypadDivide;

  case XK_F1: return keys::eF1;
  case XK_F2: return keys::eF2;
  case XK_F3: return keys::eF3;
  case XK_F4: return keys::eF4;
  case XK_F5: return keys::eF5;
  case XK_F6: return keys::eF6;
  case XK_F7: return keys::eF7;
  case XK_F8: return keys::eF8;
  case XK_F9: return keys::eF9;
  case XK_F10: return keys::eF10;
  case XK_F11: return keys::eF11;
  case XK_F12: return keys::eF12;
  case XK_F13: return keys::eF13;
  case XK_F14: return keys::eF14;
  case XK_F15: return keys::eF15;
  case XK_F16: return keys::eF16;
  case XK_F17: return keys::eF17;
  case XK_F18: return keys::eF18;
  case XK_F19: return keys::eF19;
  case XK_F20: return keys::eF20;
  case XK_F21: return keys::eF21;
  case XK_F22: return keys::eF22;
  case XK_F23: return keys::eF23;
  case XK_F24: return keys::eF24;
  case XK_F25: return keys::eF25;

  case XK_Shift_L: return keys::eLeftShift;
  case XK_Shift_R: return keys::eRightShift;
  case XK_Control_L: return keys::eLeftControl;
  case XK_Control_R: return keys::eRightControl;
  case XK_Caps_Lock: return keys::eCapsLock;
  case XK_Meta_L: return keys::eLeftAlt;
  case XK_Meta_R: return keys::eRightAlt;
  case XK_Super_L: return keys::eLeftSuper;
  case XK_Super_R: return keys::eRightSuper;

  case XK_space: return keys::eSpace;
  case XK_apostrophe: return keys::eApostrophe;
  case XK_comma: return keys::eComma;
  case XK_minus: return keys::eMinus;
  case XK_period: return keys::ePeriod;
  case XK_slash: return keys::eSlash;
  case XK_0: return keys::e0;
  case XK_1: return keys::e1;
  case XK_2: return keys::e2;
  case XK_3: return keys::e3;
  case XK_4: return keys::e4;
  case XK_5: return keys::e5;
  case XK_6: return keys::e6;
  case XK_7: return keys::e7;
  case XK_8: return keys::e8;
  case XK_9: return keys::e9;
  case XK_semicolon: return keys::eSemicolon;
  case XK_equal: return keys::eEqual;
  case XK_A: return keys::eA;
  case XK_B: return keys::eB;
  case XK_C: return keys::eC;
  case XK_D: return keys::eD;
  case XK_E: return keys::eE;
  case XK_F: return keys::eF;
  case XK_G: return keys::eG;
  case XK_H: return keys::eH;
  case XK_I: return keys::eI;
  case XK_J: return keys::eJ;
  case XK_K: return keys::eK;
  case XK_L: return keys::eL;
  case XK_M: return keys::eM;
  case XK_N: return keys::eN;
  case XK_O: return keys::eO;
  case XK_P: return keys::eP;
  case XK_Q: return keys::eQ;
  case XK_R: return keys::eR;
  case XK_S: return keys::eS;
  case XK_T: return keys::eT;
  case XK_U: return keys::eU;
  case XK_V: return keys::eV;
  case XK_W: return keys::eW;
  case XK_X: return keys::eX;
  case XK_Y: return keys::eY;
  case XK_Z: return keys::eZ;
  case XK_bracketleft: return keys::eLeftBracket;
  case XK_backslash: return keys::eBackslash;
  case XK_bracketright: return keys::eRightBracket;
  case XK_grave: return keys::eGraveAccent;
  }
  return keys::eUnknown;
} // translate_keysym

namespace error {

uint8_t s_code{0};

int handler(Display*, XErrorEvent* e) noexcept {
  s_code = e->error_code;
  return 0;
} // handler(

void grab(Display*) noexcept {
  s_code = Success;
  XSetErrorHandler(handler);
} // grab(

void release(Display* d) noexcept {
  XSync(d, False);
  XSetErrorHandler(nullptr);
} // release(

} // namespace error

} // namespace

wsi::window wsi::window::create(rect2d topleft_size, gsl::czstring title,
                                window_options opts, int screen_number,
                                std::error_code& ec) noexcept {
  window w{std::move(topleft_size)};

  char display_name[5];
  std::snprintf(display_name, 6, ":0.%d", screen_number);
  w._display = XOpenDisplay(display_name);
  if (!w._display) {
    ec.assign(EINVAL, std::system_category());
    return w;
  }

  int min, max, num;
  XDisplayKeycodes(w._display, &min, &max);
  KeySym* syms = XGetKeyboardMapping(w._display, min, max - min, &num);
  Expects(num < gsl::narrow_cast<int>(w._key_lut.size()));

  for (std::size_t i = 0; i < std::size(w._key_lut); ++i) {
    w._key_lut[i] = translate_keysym(syms[(i - min) * num]);
  }
  XFree(syms);

  int const screen = DefaultScreen(w._display);
  w._visual = DefaultVisual(w._display, screen);

  XSetWindowAttributes attrs = {};
  attrs.border_pixel = 0;
  attrs.colormap = DefaultColormap(w._display, screen);
  attrs.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
                     ButtonPressMask | ButtonReleaseMask | ExposureMask |
                     VisibilityChangeMask | PropertyChangeMask;

  error::grab(w._display);
  w._handle = XCreateWindow(
    w._display, DefaultRootWindow(w._display), w._topleft_size.offset.x,
    w._topleft_size.offset.y, w._topleft_size.extent.width,
    w._topleft_size.extent.height, 0, DefaultDepth(w._display, screen),
    InputOutput, w._visual, CWBorderPixel | CWColormap | CWEventMask, &attrs);

  error::release(w._display);
  if (error::s_code != Success) {
    std::array<char, 1024> str;
    XGetErrorText(w._display, error::s_code, str.data(), str.size());
    ec.assign(EINVAL, std::system_category());
    return w;
  }

  for (int i = 0; i < NUM_ATOMS; ++i) {
    w._atoms[i] =
      XInternAtom(w._display, atom_to_string(static_cast<Atoms>(i)), False);
  }

  XSetWMProtocols(w._display, w._handle, &w._atoms[WM_DELETE_WINDOW], 1);

  if ((opts & window_options::decorated) == window_options::decorated) {
    struct {
      unsigned long flags;
      unsigned long functions;
      unsigned long decorations;
      long input_mode;
      unsigned long status;
    } hints{2, 0, 0, 0, 0};

    XChangeProperty(w._display, w._handle, w._atoms[_MOTIF_WM_HINTS],
                    w._atoms[_MOTIF_WM_HINTS], 32, PropModeReplace,
                    reinterpret_cast<unsigned char*>(&hints),
                    sizeof(hints) / sizeof(long));
  }

  XWMHints* wm_hints = XAllocWMHints();

  wm_hints->flags = StateHint;
  wm_hints->initial_state = NormalState;

  XSetWMHints(w._display, w._handle, wm_hints);
  XFree(wm_hints);

  XSizeHints* size_hints = XAllocSizeHints();

  if ((opts & window_options::sizeable) == window_options::sizeable) {
    size_hints->flags |= (PMinSize | PMaxSize);
    size_hints->min_width = size_hints->max_width =
      w._topleft_size.extent.width;
    size_hints->min_height = size_hints->max_height =
      w._topleft_size.extent.height;
  }

  size_hints->flags |= PWinGravity;
  size_hints->win_gravity = StaticGravity;

  XSetWMNormalHints(w._display, w._handle, size_hints);
  XFree(size_hints);

  w.retitle(title);
  return w;
} // wsi::window::create

wsi::window::window(window&& other) noexcept
: impl::window<window>{std::move(other)}
, _display{other._display}
, _visual{other._visual}
, _handle{other._handle} {
  std::memcpy(_key_lut.data(), other._key_lut.data(),
              _key_lut.size() * sizeof(decltype(_key_lut)::value_type));
  std::memcpy(_atoms.data(), other._atoms.data(),
              _atoms.size() * sizeof(decltype(_atoms)::value_type));
  other._display = nullptr;
  other._handle = 0;
} // wsi::window::window(

wsi::window& wsi::window::operator=(window&& rhs) noexcept {
  if (this == &rhs) return *this;

  impl::window<window>::operator=(std::move(rhs));
  _display = rhs._display;
  _visual = rhs._visual;
  _handle = rhs._handle;
  std::memcpy(_key_lut.data(), rhs._key_lut.data(),
              _key_lut.size() * sizeof(decltype(_key_lut)::value_type));
  std::memcpy(_atoms.data(), rhs._atoms.data(),
              _atoms.size() * sizeof(decltype(_atoms)::value_type));

  rhs._display = nullptr;
  rhs._handle = 0;
  return *this;
} // wsi::window::operator=(

wsi::window::~window() noexcept {
  XDestroyWindow(_display, _handle);
  XCloseDisplay(_display);
} // wsi::window::~window()

void wsi::window::do_retitle(gsl::czstring title) noexcept {
  std::size_t const len = strlen(title);
  XmbSetWMProperties(_display, _handle, title, title, nullptr, 0, nullptr,
                     nullptr, nullptr);
  XChangeProperty(_display, _handle, _atoms[NET_WM_NAME], XA_STRING, 8,
                  PropModeReplace,
                  reinterpret_cast<unsigned char const*>(title), len);
  XChangeProperty(_display, _handle, _atoms[NET_WM_ICON_NAME], XA_STRING, 8,
                  PropModeReplace,
                  reinterpret_cast<unsigned char const*>(title), len);
} // wsi::window::do_retitle(

std::string wsi::window::do_title() const noexcept {
  Atom type;
  int format;
  unsigned long count;
  unsigned long left;
  unsigned char* title;

  XGetWindowProperty(_display, _handle, _atoms[NET_WM_NAME], 0, 65536, false,
                     XA_STRING, &type, &format, &count, &left, &title);
  return reinterpret_cast<char*>(title);
} // wsi::window::do_title()

void wsi::window::do_resize(extent2d const& size) noexcept {
  XResizeWindow(_display, _handle, size.width, size.height);
} // wsi::window::do_resize(

void wsi::window::do_reposition(offset2d const& position) noexcept {
  XMoveWindow(_display, _handle, position.x, position.y);
  _topleft_size.offset = position;
} // wsi::window::do_reposition(

void wsi::window::do_show() noexcept {
  XMapWindow(_display, _handle);
  XFlush(_display);
} // wsi::window::do_show()

void wsi::window::do_hide() noexcept {
  XUnmapWindow(_display, _handle);
  XFlush(_display);
} // wsi::window::do_hide()

void wsi::window::do_close() noexcept {
  XEvent ev = {};
  ev.type = ClientMessage;
  ev.xclient.type = ClientMessage;
  ev.xclient.display = _display;
  ev.xclient.window = _handle;
  ev.xclient.message_type = _atoms[WM_PROTOCOLS];
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = _atoms[WM_DELETE_WINDOW];

  XSendEvent(_display, _handle, False, 0, &ev);
} // wsi::window::do_close()

wsi::offset2d wsi::window::do_cursor_pos() const noexcept {
  Window root, child;
  wsi::offset2d root_pos, child_pos;
  unsigned int mask;
  XQueryPointer(_display, _handle, &root, &child, &root_pos.x, &root_pos.y,
                &child_pos.x, &child_pos.y, &mask);
  return child_pos;
} // wsi::window::do_cursor_pos()

void wsi::window::do_poll_events() noexcept {
  while (XPending(_display) > 0) {
    XEvent ev;
    XNextEvent(_display, &ev);

    switch (ev.type) {
    case KeyPress:
      if (ev.xkey.window != _handle) break;
      _keys.set(_key_lut[ev.xkey.keycode]);
      break;

    case KeyRelease:
      if (ev.xkey.window != _handle) break;
      if (XEventsQueued(_display, QueuedAfterReading)) {
        XEvent nev;
        XPeekEvent(_display, &nev);
        if (nev.type == KeyPress && nev.xkey.window == _handle &&
            nev.xkey.time == ev.xkey.time &&
            nev.xkey.keycode == ev.xkey.keycode) {
          // Next event is a KeyPress with identical window, time,
          // and keycode; thsu key wasn't physically released, so
          // consume the next KeyPress event now
          XNextEvent(_display, &ev);
          break;
        }
      }

      _keys.reset(_key_lut[ev.xkey.keycode]);
      break;

    case ButtonPress:
      if (ev.xbutton.window != _handle) break;
      switch (ev.xbutton.button) {
      case Button1: _buttons.set(buttons::e1); break;
      case Button2: _buttons.set(buttons::e2); break;
      case Button3: _buttons.set(buttons::e3); break;
      case Button4: _scroll = -1; break;
      case Button5: _scroll = 1; break;
      default:
        _buttons.set(
          static_cast<enum buttons>(ev.xbutton.button - Button1 - 4));
        break;
      }
      break;

    case ButtonRelease:
      if (ev.xbutton.window != _handle) break;
      switch (ev.xbutton.button) {
      case Button1: _buttons.reset(buttons::e1); break;
      case Button2: _buttons.reset(buttons::e2); break;
      case Button3: _buttons.reset(buttons::e3); break;
      case Button4: _scroll = 0; break;
      case Button5: _scroll = 0; break;
      default:
        _buttons.reset(
          static_cast<enum buttons>(ev.xbutton.button - Button1 - 4));
        break;
      }
      break;

    case ClientMessage:
      if (ev.xclient.message_type == None) break;
      if (ev.xclient.message_type != _atoms[WM_PROTOCOLS]) break;
      if (ev.xclient.data.l[0] == None) break;

      _closed =
        (static_cast<Atom>(ev.xclient.data.l[0]) == _atoms[WM_DELETE_WINDOW]);
      if (_closed) _on_close(this);
      break;

    case ConfigureNotify:
      if (ev.xconfigure.window != _handle) break;
      if (_topleft_size.extent.width == ev.xconfigure.width &&
          _topleft_size.extent.height == ev.xconfigure.height) {
        if (_topleft_size.offset.x == ev.xconfigure.x &&
            _topleft_size.offset.y == ev.xconfigure.y)
          break;
        _topleft_size.offset = {ev.xconfigure.x, ev.xconfigure.y};
        _on_reposition(this, _topleft_size.offset);
      } else {
        _topleft_size.extent = {ev.xconfigure.width, ev.xconfigure.height};
        _on_resize(this, _topleft_size.extent);
      }
      break;

    default: break;
    }
  }
} // wsi::window::do_poll_events()

gsl::czstring wsi::window::atom_to_string(Atoms atom) noexcept {
  switch (atom) {
#define STR(r)                                                                 \
  case r: return #r
    STR(WM_PROTOCOLS);
    STR(WM_DELETE_WINDOW);
    STR(NET_WM_NAME);
    STR(NET_WM_ICON_NAME);
    STR(_MOTIF_WM_HINTS);
#undef STR
  case NUM_ATOMS:
  default: PLAT_MARK_UNREACHABLE;
  }
  PLAT_MARK_UNREACHABLE;
}

#endif // TURF_KERNEL_LINUX
