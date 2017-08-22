#ifndef VKST_WSI_TYPES_H
#define VKST_WSI_TYPES_H

namespace wsi {

struct offset2d {
  int x{}, y{};

  constexpr offset2d() noexcept = default;
};

inline bool operator==(offset2d const& a, offset2d const& b) noexcept {
  return a.x == b.x && a.y == b.y;
}

inline bool operator!=(offset2d const& a, offset2d const& b) noexcept {
  return !(a == b);
}

inline offset2d& operator-=(offset2d& a, offset2d const& b) noexcept {
  a.x -= b.x;
  a.y -= b.y;
  return a;
}

inline offset2d operator-(offset2d a, offset2d const& b) noexcept {
  a -= b;
  return a;
}

inline offset2d& operator+=(offset2d& a, offset2d const& b) noexcept {
  a.x += b.x;
  a.y += b.y;
  return a;
}

inline offset2d operator+(offset2d a, offset2d const& b) noexcept {
  a += b;
  return a;
}

struct extent2d {
  int width{}, height{};

  constexpr extent2d() noexcept = default;
};

inline bool operator==(extent2d const& a, extent2d const& b) noexcept {
  return a.width == b.width && a.height == b.height;
}

inline bool operator!=(extent2d const& a, extent2d const& b) noexcept {
  return !(a == b);
}

struct rect2d {
  offset2d offset{};
  extent2d extent{};

  constexpr rect2d() noexcept = default;
};

inline bool operator==(rect2d const& a, rect2d const& b) noexcept {
  return a.offset == b.offset && a.extent == b.extent;
}

inline bool operator!=(rect2d const& a, rect2d const& b) noexcept {
  return !(a == b);
}

} // namespace wsi

#endif // VKST_WSI_TYPES_H
