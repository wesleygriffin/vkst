#ifndef VKST_PLAT_FILE_IO_H
#define VKST_PLAT_FILE_IO_H

#include <filesystem>
#include <system_error>
#include <vector>

namespace plat {

std::vector<char> read_file(std::experimental::filesystem::path const& path,
                            std::error_code& ec) noexcept;

} // namespace plat

#endif // VKST_PLAT_FILE_IO_H