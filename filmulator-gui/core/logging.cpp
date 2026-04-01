#include "logging.h"
#include <QDir>
#include <QStandardPaths>
#include <QString>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <vector>

namespace Filmulator {

void init_logging()
{
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(spdlog::level::trace);
  // Format: [time] [level] [thread] message
  console_sink->set_pattern("%^[%T] [%l] [t %t] %v%$");

  // File sink
  QString log_dir_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(log_dir_path);
  QString log_file_path = log_dir_path + "/filmulator.log";

  const size_t max_file_size = 5ULL * 1024ULL * 1024ULL;// 5 MB
  const size_t max_files = 3;
  auto file_sink =
    std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file_path.toStdString(), max_file_size, max_files);
  file_sink->set_level(spdlog::level::info);
  file_sink->set_pattern("[%Y-%m-%d %T] [%l] [t %t] %v");

  std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink };
  auto logger = std::make_shared<spdlog::logger>("filmulator", sinks.begin(), sinks.end());

  // Set global logger
  spdlog::set_default_logger(logger);
  spdlog::set_level(spdlog::level::trace);
  spdlog::flush_on(spdlog::level::info);

  FILM_INFO("Logging initialized. Log file: {}", log_file_path.toStdString());
}

}// namespace Filmulator
