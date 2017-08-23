#ifndef VKST_PLAT_FS_NOTIFY_H
#define VKST_PLAT_FS_NOTIFY_H

#include <plat/filesystem.h>
#include <turf/c/core.h>
#include <functional>

namespace plat {
namespace impl {

// Watch filesystem paths for changes. Works with both files and directories.
template <class D>
class fs_notify {
public:
  using watch_id = uint32_t;

  enum class actions : uint8_t {
    added,
    removed,
    modified
  }; // enum class actions

  using notify_delegate = std::function<void(
    watch_id id, plat::filesystem::path const&, actions action)>;

  watch_id add(plat::filesystem::path path, notify_delegate delegate,
               bool recursive, std::error_code& ec) noexcept {
    return reinterpret_cast<D*>(this)->do_add(std::move(path), delegate,
                                              recursive, ec);
  }

  void remove(watch_id id) noexcept {
    return reinterpret_cast<D*>(this)->do_remove(id);
  }

  void tick() noexcept { reinterpret_cast<D*>(this)->do_tick(); }
}; // class fs_notify

} // namespace impl
} // namespace plat

// clang-format off
#if TURF_TARGET_WIN32
#  include <plat/fs_notify_win32.h>
#elif TURF_KERNEL_LINUX
#  include <plat/fs_notify_linux.h>
#else
#  error "Unsupported platform"
#endif
// clang-format on

#endif // VKST_PLAT_FS_NOTIFY_H