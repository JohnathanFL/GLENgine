#include "Logger.hpp"
std::unique_ptr<Logger> Logger::instance = std::make_unique<Logger>();
