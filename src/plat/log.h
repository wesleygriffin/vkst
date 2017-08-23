#ifndef VKST_PLAT_LOG_H
#define VKST_PLAT_LOG_H

#include <plat/filesystem.h>
#include <turf/c/core.h>
#include <gsl.h>
#include <string>

namespace plat {

// Initialize logging with output to a given path.
// Must be called before logging output will occur.
void init_logging(plat::filesystem::path logfile, std::error_code& ec) noexcept;

enum class log_severities : uint8_t {
  trace = 1,
  debug,
  info,
  warn,
  error,
  fatal,
  none
};

inline constexpr gsl::czstring to_string(log_severities severity) noexcept {
  switch(severity) {
  case log_severities::trace: return "TRACE";
  case log_severities::debug: return "DEBUG";
  case log_severities::info: return "INFO";
  case log_severities::warn: return "WARN";
  case log_severities::error: return "ERROR";
  case log_severities::fatal: return "FATAL";
  case log_severities::none: return "NONE";
  }
}

void log(log_severities severity, gsl::czstring fmt, ...) noexcept;

#define LOG_TRACE(...) ::plat::log(::plat::log_severities::trace, __VA_ARGS__)
#define LOG_DEBUG(...) ::plat::log(::plat::log_severities::debug, __VA_ARGS__)
#define LOG_INFO(...) ::plat::log(::plat::log_severities::info, __VA_ARGS__)
#define LOG_WARN(...) ::plat::log(::plat::log_severities::warn, __VA_ARGS__)
#define LOG_ERROR(...) ::plat::log(::plat::log_severities::error, __VA_ARGS__)
#define LOG_FATAL(...) ::plat::log(::plat::log_severities::fatal, __VA_ARGS__)

#define LOG_ENTER                                                              \
  ::plat::log(::plat::log_severities::trace, "enter: %s", __func__)
#define LOG_LEAVE                                                              \
  ::plat::log(::plat::log_severities::trace, "leave: %s", __func__)

} // namespace plat

#endif // VKST_PLAT_LOG_H