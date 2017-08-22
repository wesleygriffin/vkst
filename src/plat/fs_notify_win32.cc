#include "fs_notify.h"

#if TURF_TARGET_WIN32
static void CALLBACK WatchCallback(DWORD dwErrorCode,
                                   DWORD dwNumberOfBytesTransferred,
                                   LPOVERLAPPED lpOverlapped) noexcept {
  if (dwNumberOfBytesTransferred == 0) return;

  auto w = reinterpret_cast<plat::fs_notify::watch*>(lpOverlapped);
  if (dwErrorCode != ERROR_SUCCESS) {
    if (!w->stop) w->refresh();
    return;
  }

  std::size_t offset = 0;
  PFILE_NOTIFY_INFORMATION n;
  TCHAR fn[MAX_PATH];

  do {
    n = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(&w->buffer[offset]);
    offset += n->NextEntryOffset;

#ifdef UNICODE
    lstrcpynW(fn, n->FileName,
              std::min(MAX_PATH, n->FileNameLength / sizeof(WCHAR) + 1));
#else  // UNICODE
    int count = WideCharToMultiByte(CP_ACP, 0, n->FileName,
                                    n->FileNameLength / sizeof(WCHAR), fn,
                                    MAX_PATH - 1, NULL, NULL);
    fn[count] = TEXT('\0');
#endif // UNICODE

    if (std::experimental::filesystem::is_directory(w->path) ||
        (w->path.filename() == std::experimental::filesystem::path{fn})) {
      w->notify(fn, n->Action);
    }
  } while (n->NextEntryOffset != 0);

  if (!w->stop) w->refresh();
} // WatchCallback

bool plat::fs_notify::watch::refresh(bool clear) noexcept {
  return (ReadDirectoryChangesW(
            handle, buffer.data(), gsl::narrow_cast<DWORD>(buffer.size()),
            recursive,
            FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE |
              FILE_NOTIFY_CHANGE_FILE_NAME,
            NULL, &overlapped, clear ? 0 : WatchCallback) != 0);
} // plat::fs_notify::watch::refresh

void plat::fs_notify::watch::notify(
  std::experimental::filesystem::path changed_path, DWORD action) noexcept {
  actions act;
  switch(action) {
  case FILE_ACTION_RENAMED_NEW_NAME: // FALLTHROUGH
  case FILE_ACTION_ADDED:
    act = actions::added;
    break;
  case FILE_ACTION_RENAMED_OLD_NAME: // FALLTHROUGH
  case FILE_ACTION_REMOVED:
    act = actions::removed;
    break;
  case FILE_ACTION_MODIFIED:
    act = actions::modified;
    break;
  }
  delegate(id, changed_path, act);
} // plat::fs_notify::watch::notify

plat::fs_notify::watch::~watch() noexcept {
  stop = true;
  CancelIo(handle);
  refresh(true);
  if (!HasOverlappedIoCompleted(&overlapped)) SleepEx(5, TRUE);
  if (overlapped.hEvent != INVALID_HANDLE_VALUE) CloseHandle(overlapped.hEvent);
  if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
} // plat::fs_notify::watch::~watch

plat::fs_notify::watch_id
plat::fs_notify::do_add(std::experimental::filesystem::path path,
                        impl::fs_notify<fs_notify>::notify_delegate delegate,
                        bool recursive, std::error_code& ec) noexcept {
  auto w = gsl::make_unique<watch>(std::move(path), delegate, recursive);
  if (!w) {
    ec.assign(ENOMEM, std::generic_category());
    return UINT32_MAX;
  }

  auto const parent = w->path.parent_path();
  w->handle = CreateFile(
    parent.string().c_str(), FILE_LIST_DIRECTORY,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
  if (w->handle == INVALID_HANDLE_VALUE) {
    ec.assign(gsl::narrow_cast<int>(GetLastError()), std::system_category());
    return UINT32_MAX;
  }

  w->overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  w->id = gsl::narrow_cast<watch_id>(_watches.size());
  if (!w->refresh()) {
    ec.assign(gsl::narrow_cast<int>(GetLastError()), std::system_category());
    return UINT32_MAX;
  }

  _watches.push_back(std::move(w));
  return gsl::narrow_cast<watch_id>(_watches.size() - 1);
} // plat::fs_notify::do_add

void plat::fs_notify::do_remove(watch_id id) noexcept {
  _watches.erase(_watches.begin() + id);
} // plat::fs_notify::do_remove

void plat::fs_notify::do_tick() noexcept {
  MsgWaitForMultipleObjectsEx(0, NULL, 0, QS_ALLINPUT, MWMO_ALERTABLE);
} // plat::fs_notify::do_tick

#endif // TURF_TARGET_WIN32
