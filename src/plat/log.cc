#include "log.h"
#include "file_handle.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#include <array>
#include <chrono>

static plat::file_handle s_fh;

void plat::init_logging(std::experimental::filesystem::path logfile,
                        std::error_code& ec) noexcept {
  if (!s_fh) {
    s_fh = file_handle::open(logfile, file_handle::open_modes::write, ec);
  }
}

void plat::log(log_severities severity, gsl::czstring fmt, ...) noexcept {
  if (!s_fh) return;

  using namespace std::chrono;
  auto const now = system_clock::now();
  auto const ms = time_point_cast<milliseconds>(now);
  auto const s = time_point_cast<seconds>(now);
  auto const t = system_clock::to_time_t(ms);

  std::tm tm;
#if TURF_COMPILER_MSVC
  localtime_s(&tm, &t);
#else
  localtime_r(&t, &tm);
#endif

  std::array<char, 1024> str;
  auto pos = std::strftime(str.data(), str.size(), "[%Y-%m-%d %H:%M:%S.", &tm);
  pos +=
    stbsp_snprintf(str.data() + pos, gsl::narrow_cast<int>(str.size() - pos),
                   "%04lld] [%-5s] ", (ms - s).count(), to_string(severity));

  va_list args;
  va_start(args, fmt);
  stbsp_vsnprintf(str.data() + pos, gsl::narrow_cast<int>(str.size() - pos),
                  fmt, args);
  va_end(args);

  std::fputs(str.data(), s_fh);
  std::fputs("\n", s_fh);
  std::fflush(s_fh);
} // plat::log
