#ifndef VKST_PLAT_FS_NOTIFY_WIN32_H
#define VKST_PLAT_FS_NOTIFY_WIN32_H

#ifndef VKST_PLAT_FS_NOTIFY_H
#error "Include fs_notify.h only"
#endif

#include "fs_notify.h"
#include <gsl.h>
#include <vector>

namespace plat {

class fs_notify final : public impl::fs_notify<fs_notify> {
public:
  class watch {
  public:
    OVERLAPPED overlapped{};
    std::experimental::filesystem::path path{};
    notify_delegate delegate{};
    bool recursive{false};
    HANDLE handle{INVALID_HANDLE_VALUE};
    std::array<BYTE, 32 * 1024> buffer{};
    watch_id id{UINT32_MAX};
    bool stop{false};

    bool refresh(bool clear = false) noexcept;
    void notify(std::experimental::filesystem::path changed_path,
                DWORD action) noexcept;

    watch(std::experimental::filesystem::path p, notify_delegate d,
          bool r) noexcept
    : path{std::move(p)}, delegate{std::move(d)}, recursive{r} {}

    watch() = default;
    watch(watch const&) = delete;
    watch& operator=(watch const&) = delete;
    ~watch() noexcept;
  }; // struct watch

private:
  std::vector<gsl::unique_ptr<watch>> _watches;

  watch_id do_add(std::experimental::filesystem::path path,
                  impl::fs_notify<fs_notify>::notify_delegate delegate,
                  bool recursive, std::error_code& ec) noexcept;

  void do_remove(watch_id id) noexcept;

  void do_tick() noexcept;

  friend class impl::fs_notify<fs_notify>;
}; // class fs_notify

} // namespace plat

#endif // VKST_PLAT_FS_NOTIFY_WIN32_H