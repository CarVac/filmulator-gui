#ifndef FILMULATOR_LOGGING_H
#define FILMULATOR_LOGGING_H

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

namespace Filmulator {

void init_logging();

}// namespace Filmulator

#define FILM_TRACE(...) spdlog::trace(__VA_ARGS__)
#define FILM_DEBUG(...) spdlog::debug(__VA_ARGS__)
#define FILM_INFO(...) spdlog::info(__VA_ARGS__)
#define FILM_WARN(...) spdlog::warn(__VA_ARGS__)
#define FILM_ERROR(...) spdlog::error(__VA_ARGS__)
#define FILM_CRITICAL(...) spdlog::critical(__VA_ARGS__)

#endif// FILMULATOR_LOGGING_H
