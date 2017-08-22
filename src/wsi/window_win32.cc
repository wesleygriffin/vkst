#include "window.h"

#if TURF_TARGET_WIN32
#include <algorithm>
#include <shellapi.h>
#include <type_traits>

namespace {

wsi::keys translate_keycode(WPARAM keyCode) {
  using namespace wsi;
  switch (keyCode) {
  case VK_BACK: return keys::eBackspace;
  case VK_TAB: return keys::eTab;
  case VK_RETURN: return keys::eEnter;

  case VK_PAUSE: return keys::ePause;
  case VK_CAPITAL: return keys::eCapsLock;

  case VK_ESCAPE: return keys::eEscape;
  case VK_SPACE: return keys::eSpace;
  case VK_PRIOR: return keys::ePageUp;
  case VK_NEXT: return keys::ePageDown;
  case VK_END: return keys::eEnd;
  case VK_HOME: return keys::eHome;
  case VK_LEFT: return keys::eLeft;
  case VK_RIGHT: return keys::eRight;
  case VK_DOWN: return keys::eDown;

  case VK_SNAPSHOT: return keys::ePrintScreen;
  case VK_INSERT: return keys::eInsert;
  case VK_DELETE: return keys::eDelete;

  case 0x30: return keys::e0;
  case 0x31: return keys::e1;
  case 0x32: return keys::e2;
  case 0x33: return keys::e3;
  case 0x34: return keys::e4;
  case 0x35: return keys::e5;
  case 0x36: return keys::e6;
  case 0x37: return keys::e7;
  case 0x38: return keys::e8;
  case 0x39: return keys::e9;
  case 0x41: return keys::eA;
  case 0x42: return keys::eB;
  case 0x43: return keys::eC;
  case 0x44: return keys::eD;
  case 0x45: return keys::eE;
  case 0x46: return keys::eF;
  case 0x47: return keys::eG;
  case 0x48: return keys::eH;
  case 0x49: return keys::eI;
  case 0x4A: return keys::eJ;
  case 0x4B: return keys::eK;
  case 0x4C: return keys::eL;
  case 0x4D: return keys::eM;
  case 0x4E: return keys::eN;
  case 0x4F: return keys::eO;
  case 0x50: return keys::eP;
  case 0x51: return keys::eQ;
  case 0x52: return keys::eR;
  case 0x53: return keys::eS;
  case 0x54: return keys::eT;
  case 0x55: return keys::eU;
  case 0x56: return keys::eV;
  case 0x57: return keys::eW;
  case 0x58: return keys::eX;
  case 0x59: return keys::eY;
  case 0x5A: return keys::eZ;

  case VK_LWIN: return keys::eLeftSuper;
  case VK_RWIN: return keys::eRightSuper;

  case VK_NUMPAD0: return keys::eKeypad0;
  case VK_NUMPAD1: return keys::eKeypad1;
  case VK_NUMPAD2: return keys::eKeypad2;
  case VK_NUMPAD3: return keys::eKeypad3;
  case VK_NUMPAD4: return keys::eKeypad4;
  case VK_NUMPAD5: return keys::eKeypad5;
  case VK_NUMPAD6: return keys::eKeypad6;
  case VK_NUMPAD7: return keys::eKeypad7;
  case VK_NUMPAD8: return keys::eKeypad8;
  case VK_NUMPAD9: return keys::eKeypad9;
  case VK_MULTIPLY: return keys::eKeypadMultiply;
  case VK_ADD: return keys::eKeypadAdd;
  case VK_SUBTRACT: return keys::eKeypadSubtract;
  case VK_DECIMAL: return keys::eKeypadDecimal;
  case VK_DIVIDE: return keys::eKeypadDivide;

  case VK_F1: return keys::eF1;
  case VK_F2: return keys::eF2;
  case VK_F3: return keys::eF3;
  case VK_F4: return keys::eF4;
  case VK_F5: return keys::eF5;
  case VK_F6: return keys::eF6;
  case VK_F7: return keys::eF7;
  case VK_F8: return keys::eF8;
  case VK_F9: return keys::eF9;
  case VK_F10: return keys::eF10;
  case VK_F11: return keys::eF11;
  case VK_F12: return keys::eF12;
  case VK_F13: return keys::eF13;
  case VK_F14: return keys::eF14;
  case VK_F15: return keys::eF15;
  case VK_F16: return keys::eF16;
  case VK_F17: return keys::eF17;
  case VK_F18: return keys::eF18;
  case VK_F19: return keys::eF19;
  case VK_F20: return keys::eF20;
  case VK_F21: return keys::eF21;
  case VK_F22: return keys::eF22;
  case VK_F23: return keys::eF23;
  case VK_F24: return keys::eF24;

  case VK_NUMLOCK: return keys::eNumLock;
  case VK_SCROLL: return keys::eScrollLock;

  case VK_SHIFT: return keys::eLeftShift;
  case VK_CONTROL: return keys::eLeftControl;
  case VK_LSHIFT: return keys::eLeftShift;
  case VK_RSHIFT: return keys::eRightShift;
  case VK_LCONTROL: return keys::eLeftControl;
  case VK_RCONTROL: return keys::eRightControl;
  case VK_LMENU: return keys::eLeftAlt;
  case VK_RMENU: return keys::eRightAlt;
  }

  return keys::eUnknown;
} // translate_keycode

} // namespace

wsi::window wsi::window::create(rect2d topleft_size, gsl::czstring title,
                                window_options opts,
                                std::error_code& ec) noexcept {
  window w{std::move(topleft_size)};
  w._instance = GetModuleHandle(NULL);
  if (!w._instance) {
    ec.assign(gsl::narrow_cast<int>(GetLastError()), std::system_category());
    return w;
  }

  for (std::size_t i = 0; i < std::size(w._key_lut); ++i) {
    w._key_lut[i] = translate_keycode(i);
  }

  char exePath[MAX_PATH];
  GetModuleFileName(NULL, exePath, MAX_PATH);

  WNDCLASS wc = {};
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = window::WndProc;
  wc.hInstance = w._instance;
  wc.hIcon = ExtractIcon(w._instance, exePath, 0);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
  wc.lpszClassName = TEXT("wsi_Window");

  ATOM const atom = RegisterClass(&wc);
  if (atom == 0) {
    int err = GetLastError();
    if (err != ERROR_CLASS_ALREADY_EXISTS) {
      ec.assign(err, std::system_category());
      return w;
    }
  }

  DWORD dwStyle = WS_OVERLAPPEDWINDOW;
  if ((opts & window_options::sizeable) != window_options::sizeable) {
    dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
  }

  RECT rect;
  SetRect(&rect, 0, 0, w._topleft_size.extent.width,
          w._topleft_size.extent.height);
  AdjustWindowRect(&rect, dwStyle, FALSE);

  auto handle =
    CreateWindow(wc.lpszClassName, title, dwStyle, w._topleft_size.offset.x,
                 w._topleft_size.offset.y, (rect.right - rect.left),
                 (rect.bottom - rect.top), nullptr, nullptr, wc.hInstance, &w);
  if (!handle || !w._handle) {
    ec.assign(static_cast<int>(GetLastError()), std::system_category());
    return w;
  }

  if ((opts & window_options::fullscreen_windowed) ==
      window_options::fullscreen_windowed) {
    MONITORINFO mi = {sizeof(mi)};
    GetMonitorInfo(MonitorFromWindow(w._handle, MONITOR_DEFAULTTOPRIMARY), &mi);

    SetWindowLong(w._handle, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
    SetWindowPos(w._handle, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                 mi.rcMonitor.right - mi.rcMonitor.left,
                 mi.rcMonitor.bottom - mi.rcMonitor.top,
                 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
  } else if ((opts & window_options::decorated) != window_options::decorated) {
    SetWindowLong(w._handle, GWL_STYLE, 0);
  }

  w.retitle(title);
  return w;
} // wsi::window::create

wsi::window::window(window&& other) noexcept
: impl::window<window>{std::move(other)}
, _instance{other._instance}
, _handle{other._handle} {
  std::memcpy(_key_lut.data(), other._key_lut.data(),
              _key_lut.size() * sizeof(decltype(_key_lut)::value_type));
  SetWindowLongPtr(_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  other._instance = 0;
  other._handle = 0;
} // wsi::window::window(

wsi::window& wsi::window::operator=(window&& rhs) noexcept {
  if (this == &rhs) return *this;

  impl::window<window>::operator=(std::move(rhs));
  _instance = rhs._instance;
  _handle = rhs._handle;
  std::memcpy(_key_lut.data(), rhs._key_lut.data(),
              _key_lut.size() * sizeof(decltype(_key_lut)::value_type));
  SetWindowLongPtr(_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

  rhs._instance = 0;
  rhs._handle = 0;
  return *this;
} // wsi::window::operator=(

wsi::window::~window() noexcept {
  DestroyWindow(_handle);
} // wsi::window::~window()

void wsi::window::do_retitle(gsl::czstring title) noexcept {
  SetWindowText(_handle, title);
} // wsi::window::do_retitle(

std::string wsi::window::do_title() const noexcept {
  TCHAR title[1024];
  GetWindowText(_handle, title, 1024);
  return title;
} // wsi::window::do_title()

void wsi::window::do_resize(extent2d const& size) noexcept {
  RECT rect;
  SetRect(&rect, 0, 0, size.width, size.height);
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

  SetWindowPos(_handle, HWND_NOTOPMOST, 0, 0, rect.right - rect.left,
               rect.bottom - rect.top, SWP_NOMOVE | SWP_NOREPOSITION);
  _topleft_size.extent = size;
} // wsi::window::do_resize(

void wsi::window::do_reposition(offset2d const& position) noexcept {
  SetWindowPos(_handle, HWND_NOTOPMOST, position.x, position.y, 0, 0,
               SWP_NOSIZE);
  _topleft_size.offset = position;
} // wsi::window::do_reposition(

void wsi::window::do_show() noexcept {
  ShowWindow(_handle, SW_SHOW);
} // wsi::window::do_show()

void wsi::window::do_hide() noexcept {
  ShowWindow(_handle, SW_HIDE);
} // wsi::window::do_hide()

void wsi::window::do_close() noexcept {
  SendMessage(_handle, WM_CLOSE, 0, 0);
} // wsi::window::do_close()

wsi::offset2d wsi::window::do_cursor_pos() const noexcept {
  POINT pos;
  GetCursorPos(&pos);
  return {pos.x, pos.y};
} // wsi::window::do_cursor_pos()

void wsi::window::do_poll_events() noexcept {
  MSG msg = {};
  msg.message = WM_NULL;

  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
} // wsi::window::do_poll_events()

LRESULT wsi::window::wnd_proc(UINT uMsg, WPARAM wParam,
                              LPARAM lParam) noexcept {
  switch (uMsg) {
  case WM_KEYDOWN: _keys.set(_key_lut[wParam]); return 0;
  case WM_KEYUP: _keys.reset(_key_lut[wParam]); return 0;
  case WM_LBUTTONDOWN: _buttons.set(buttons::e1); return 0;
  case WM_MBUTTONDOWN: _buttons.set(buttons::e2); return 0;
  case WM_RBUTTONDOWN: _buttons.set(buttons::e3); return 0;
  case WM_XBUTTONDOWN:
    _buttons.set((GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? buttons::e4
                                                          : buttons::e5);
    return 0;
  case WM_LBUTTONUP: _buttons.reset(buttons::e1); return 0;
  case WM_MBUTTONUP: _buttons.reset(buttons::e2); return 0;
  case WM_RBUTTONUP: _buttons.reset(buttons::e3); return 0;
  case WM_XBUTTONUP:
    _buttons.reset((GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? buttons::e4
                                                            : buttons::e5);
    return 0;
  case WM_MOUSEWHEEL:
    _scroll += GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
    return 0;
  case WM_SIZE:
    _topleft_size.extent = {LOWORD(lParam), HIWORD(lParam)};
    _on_resize(this, _topleft_size.extent);
    return 0;
  case WM_MOVE:
    _topleft_size.offset = {LOWORD(lParam), HIWORD(lParam)};
    _on_reposition(this, _topleft_size.offset);
    return 0;
  case WM_CLOSE:
    _closed = true;
    _on_close(this);
    return 0;
  case WM_DESTROY: ::PostQuitMessage(0); break;
  }

  return ::DefWindowProc(_handle, uMsg, wParam, lParam);
} // wsi::window::wndProc(

LRESULT CALLBACK wsi::window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                      LPARAM lParam) noexcept {
  if (uMsg == WM_NCCREATE) {
    auto win = reinterpret_cast<window*>(
      reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams);
    if (win) {
      win->_handle = hWnd;
      SetWindowLongPtr(win->_handle, GWLP_USERDATA,
                       reinterpret_cast<LONG_PTR>(win));
    }
  }

  auto win = reinterpret_cast<window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
  if (win) return win->wnd_proc(uMsg, wParam, lParam);
  return DefWindowProc(hWnd, uMsg, wParam, lParam);
} // wsi::gWindow::wndProc(
#endif // TURF_TARGET_WIN32