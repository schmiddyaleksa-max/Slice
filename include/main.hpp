#pragma once

#include "scotland2/shared/modloader.h"

#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"

Logger& getLogger();

#define LOG_INFO(...) getLogger().info(__VA_ARGS__)
#define LOG_DEBUG(...)
// #define LOG_DEBUG(...) getLogger().debug(__VA_ARGS__)
