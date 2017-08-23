#ifndef VKST_PLAT_FILESYSTEM_H
#define VKST_PLAT_FILESYSTEM_H

#include "plat/plat_config.h"

#if PLAT_STD_FILESYSTEM
#  include <filesystem>
#elif PLAT_EXP_FILESYSTEM
#  include <experimental/filesystem>
#endif

namespace plat {

#if PLAT_STD_FILESYSTEM_NS
namespace filesystem = std::filesystem;
#elif PLAT_EXP_FILESYSTEM_NS
namespace filesystem = std::experimental::filesystem;
#endif

} // namespace plat

#endif // VKST_PLAT_FILESYSTEM_H