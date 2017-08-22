#ifndef VKST_WSI_INPUT_H
#define VKST_WSI_INPUT_H

#include <gsl.h>
#include <wsi/types.h>
#include <bitset>

namespace wsi {

enum class keys {
  eUnknown = 0,

  eSpace = 32,
  eApostrophe = 39,
  eComma = 44,
  eMinus = 45,
  ePeriod = 46,
  eSlash = 47,
  e0 = 48,
  e1 = 49,
  e2 = 50,
  e3 = 51,
  e4 = 52,
  e5 = 53,
  e6 = 54,
  e7 = 55,
  e8 = 56,
  e9 = 57,
  eSemicolon = 59,
  eEqual = 61,
  eA = 65,
  eB = 66,
  eC = 67,
  eD = 68,
  eE = 69,
  eF = 70,
  eG = 71,
  eH = 72,
  eI = 73,
  eJ = 74,
  eK = 75,
  eL = 76,
  eM = 77,
  eN = 78,
  eO = 79,
  eP = 80,
  eQ = 81,
  eR = 82,
  eS = 83,
  eT = 84,
  eU = 85,
  eV = 86,
  eW = 87,
  eX = 88,
  eY = 89,
  eZ = 90,
  eLeftBracket = 91,
  eBackslash = 92,
  eRightBracket = 93,
  eGraveAccent = 96,

  eEscape = 156,
  eEnter = 157,
  eTab = 158,
  eBackspace = 159,
  eInsert = 160,
  eDelete = 161,
  eRight = 162,
  eLeft = 163,
  eDown = 164,
  eUp = 165,
  ePageUp = 166,
  ePageDown = 167,
  eHome = 168,
  eEnd = 169,

  eCapsLock = 180,
  eScrollLock = 181,
  eNumLock = 182,
  ePrintScreen = 183,
  ePause = 184,

  eF1 = 190,
  eF2 = 191,
  eF3 = 192,
  eF4 = 193,
  eF5 = 194,
  eF6 = 195,
  eF7 = 196,
  eF8 = 197,
  eF9 = 198,
  eF10 = 199,
  eF11 = 200,
  eF12 = 201,
  eF13 = 202,
  eF14 = 203,
  eF15 = 204,
  eF16 = 205,
  eF17 = 206,
  eF18 = 207,
  eF19 = 208,
  eF20 = 209,
  eF21 = 210,
  eF22 = 211,
  eF23 = 212,
  eF24 = 213,
  eF25 = 214,

  eKeypad0 = 220,
  eKeypad1 = 221,
  eKeypad2 = 222,
  eKeypad3 = 223,
  eKeypad4 = 224,
  eKeypad5 = 225,
  eKeypad6 = 226,
  eKeypad7 = 227,
  eKeypad8 = 228,
  eKeypad9 = 229,
  eKeypadDecimal = 230,
  eKeypadDivide = 231,
  eKeypadMultiply = 232,
  eKeypadSubtract = 233,
  eKeypadAdd = 234,
  eKeypadEnter = 235,
  eKeypadEqual = 236,

  eLeftShift = 240,
  eLeftControl = 241,
  eLeftAlt = 242,
  eLeftSuper = 243,
  eRightShift = 244,
  eRightControl = 245,
  eRightAlt = 246,
  eRightSuper = 247,
  eMenu = 248,
}; // enum class keys

class keyset final {
  std::bitset<256> _keys;

public:
  using reference = decltype(_keys)::reference;

  bool all() const { return _keys.all(); }
  bool any() const { return _keys.any(); }
  bool none() const { return _keys.none(); }

  constexpr bool operator[](keys key) const {
    return _keys[static_cast<std::size_t>(key)];
  }

  reference operator[](keys key) {
    return _keys[static_cast<std::size_t>(key)];
  }

  keyset& set(keys key, bool value = true) noexcept {
    _keys[static_cast<std::size_t>(key)] = value;
    return *this;
  }

  keyset& reset(keys key) noexcept {
    _keys[static_cast<std::size_t>(key)] = false;
    return *this;
  }
}; // class keyset

enum class buttons {
  e1 = 1,
  e2,
  e3,
  e4,
  e5,
  e6,
  e7,
  e8,
  e9,
  e10,
}; // enum class buttons

class buttonset final {
  std::bitset<32> _buttons;

public:
  using reference = decltype(_buttons)::reference;

  bool all() const { return _buttons.all(); }
  bool any() const { return _buttons.any(); }
  bool none() const { return _buttons.none(); }

  constexpr bool operator[](buttons button) const {
    return _buttons[static_cast<std::size_t>(button)];
  }

  reference operator[](buttons button) {
    return _buttons[static_cast<std::size_t>(button)];
  }

  buttonset& set(buttons button, bool value = true) noexcept {
    _buttons[static_cast<std::size_t>(button)] = value;
    return *this;
  }

  buttonset& reset(buttons button) noexcept {
    _buttons[static_cast<std::size_t>(button)] = false;
    return *this;
  }
}; // class buttonset

class window;

class input final {
public:
  input(gsl::not_null<window*> win) noexcept;

  bool key_pressed(keys key) const noexcept {
    return !_prev_keys[key] && _curr_keys[key];
  }

  bool key_released(keys key) const noexcept {
    return _prev_keys[key] && !_curr_keys[key];
  }

  bool key_down(keys key) const noexcept {
    return _prev_keys[key] && _curr_keys[key];
  }

  bool button_pressed(buttons button) const noexcept {
    return !_prev_buttons[button] && _curr_buttons[button];
  }

  bool button_released(buttons button) const noexcept {
    return _prev_buttons[button] && !_curr_buttons[button];
  }

  bool button_down(buttons button) const noexcept {
    return _prev_buttons[button] && _curr_buttons[button];
  }

  int prev_scroll() const noexcept { return _prev_scroll; }
  int curr_scroll() const noexcept { return _curr_scroll; }

  int scroll_delta() const noexcept { return _curr_scroll - _prev_scroll; }

  offset2d cursor_pos() const noexcept;

  offset2d prev_cursor_pos() const noexcept { return _prev_pos; }
  offset2d curr_cursor_pos() const noexcept { return _curr_pos; }

  offset2d cursor_delta() const noexcept { return _curr_pos - _prev_pos; }

  void tick() noexcept;

private:
  window* _win;
  keyset _prev_keys, _curr_keys;
  buttonset _prev_buttons, _curr_buttons;
  int _prev_scroll, _curr_scroll;
  offset2d _prev_pos, _curr_pos;
}; // class input

} // namespace wsi

#endif // VKST_WSI_INPUT_H
