#ifndef VKST_PLAT_FS_NOTIFY_LINUX_H
#define VKST_PLAT_FS_NOTIFY_LINUX_H

#ifndef VKST_PLAT_FS_NOTIFY_H
#error "Include fs_notify.h only"
#endif

#include "fs_notify.h"
#include <gsl.h>
#include <vector>

namespace plat {

class fs_notify final : public impl::fs_notify<fs_notify> {
public:

private:
  //std::vector<gsl::unique_ptr<watch>> _watches;

  watch_id do_add(plat::filesystem::path path,
                  impl::fs_notify<fs_notify>::notify_delegate delegate,
                  bool recursive, std::error_code& ec) noexcept;

  void do_remove(watch_id id) noexcept;

  void do_tick() noexcept;

  friend class impl::fs_notify<fs_notify>;
}; // class fs_notify

} // namespace plat

#endif // VKST_PLAT_FS_NOTIFY_LINUX_H

