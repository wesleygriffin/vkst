#include "file_io.h"
#include "file_handle.h"

std::vector<char>
plat::read_file(std::experimental::filesystem::path const& path,
                std::error_code& ec) noexcept {
  auto fh = plat::file_handle::open(path, plat::file_handle::open_modes::read, ec);
  if (ec) return {};

  std::fseek(fh, 0, SEEK_END);
  std::vector<char> bytes(std::ftell(fh));
  std::rewind(fh);

  auto const nread = std::fread(bytes.data(), sizeof(char), bytes.size(), fh);
  if (ferror(fh) || nread != bytes.size()) {
    ec.assign(EIO, std::generic_category());
  }

  return bytes;
} // read_file

