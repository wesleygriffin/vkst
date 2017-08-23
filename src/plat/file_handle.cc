#include "file_handle.h"
#include <cstdio>

plat::file_handle plat::file_handle::open(plat::filesystem::path const& path,
                                          open_modes mode,
                                          std::error_code& ec) noexcept {
  std::string mode_str = "";
  if ((mode & open_modes::read) == open_modes::read) mode_str += "r";
  if ((mode & open_modes::write) == open_modes::write) mode_str += "w";
  if ((mode & open_modes::append) == open_modes::append) mode_str += "a";
  mode_str += "b";

  file_handle fh{std::fopen(path.string().c_str(), mode_str.c_str())};

  if (!fh) ec.assign(EIO, std::generic_category());
  else ec.clear();
  return fh;
} // plat::file_handle::open
