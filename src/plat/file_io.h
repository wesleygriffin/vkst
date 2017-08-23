#ifndef VKST_PLAT_FILE_IO_H
#define VKST_PLAT_FILE_IO_H

#include <plat/filesystem.h>
#include <system_error>
#include <vector>

namespace plat {

// Read the contents of a file.
std::vector<char> read_file(plat::filesystem::path const& path,
                            std::error_code& ec) noexcept;

} // namespace plat

#endif // VKST_PLAT_FILE_IO_H