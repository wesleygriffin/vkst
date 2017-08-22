#ifndef VKST_PLAT_FILE_HANDLE_H
#define VKST_PLAT_FILE_HANDLE_H

#include <turf/c/core.h>
#include <gsl.h>
#include <filesystem>

namespace plat {

//! \brief A RAII wrapper around the C stdio file handle
class file_handle {
public:
  enum class open_modes : uint8_t {
    read = 0x1,
    write = 0x2,
    append = 0x4,
  }; // enum class open_modes

  static file_handle open(std::experimental::filesystem::path const& path,
                          open_modes mode, std::error_code& ec) noexcept;

  file_handle() noexcept = default;

  std::FILE* get() noexcept { return _handle.get(); }
  operator std::FILE*() noexcept { return get(); }

  explicit operator bool() noexcept { return static_cast<bool>(_handle); }

  void reset() noexcept { _handle.reset(); }

private:
  file_handle(FILE* handle) noexcept : _handle{handle, &std::fclose} {}

  using file_ptr = gsl::unique_ptr<std::FILE, decltype(&std::fclose)>;
  file_ptr _handle{nullptr, &std::fclose};
}; // class file_handle

inline constexpr auto operator|(file_handle::open_modes a,
                                file_handle::open_modes b) noexcept {
  using U = std::underlying_type_t<file_handle::open_modes>;
  return static_cast<file_handle::open_modes>(static_cast<U>(a) |
                                              static_cast<U>(b));
}

inline constexpr auto operator&(file_handle::open_modes a,
                                file_handle::open_modes b) noexcept {
  using U = std::underlying_type_t<file_handle::open_modes>;
  return static_cast<file_handle::open_modes>(static_cast<U>(a) &
                                              static_cast<U>(b));
}

} // namespace plat

#endif // VKST_PLAT_FILE_HANDLE_H