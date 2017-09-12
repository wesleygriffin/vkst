#include "fs_notify.h"
#include "core.h"

#if TURF_KERNEL_LINUX

plat::fs_notify::watch_id
plat::fs_notify::do_add(plat::filesystem::path path,
                        impl::fs_notify<fs_notify>::notify_delegate delegate,
                        bool recursive, std::error_code& ec) noexcept {
  return 0;
} // plat::fs_notify::do_add

void plat::fs_notify::do_remove(watch_id id) noexcept {
} // plat::fs_notify::do_remove

void plat::fs_notify::do_tick() noexcept {
} // plat::fs_notify::do_tick

#endif // TURF_KERNEL_LINUX

