/**
 * @file CrowLogHandler.cpp
 *
 * @author Dan Keenan
 * @date 5/25/25
 * @copyright GPL-3.0-or-later
 * Copyright (C) 2024-2025 Dan Keenan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <sacnloggerlib/CrowLogHandler.h>
#include <spdlog/spdlog.h>

namespace sacnlogger
{
    void CrowLogHandler::log(std::string message, crow::LogLevel crowLogLevel)
    {
        // Convert Crow's log levels to spdlog's.
        const spdlog::level::level_enum level = [crowLogLevel]()
        {
            switch (crowLogLevel)
            {
            case crow::LogLevel::Debug:
            case crow::LogLevel::Info:
                // Crow uses Info more often than it should...
                return spdlog::level::level_enum::debug;
            case crow::LogLevel::Warning:
                return spdlog::level::level_enum::warn;
            case crow::LogLevel::Error:
                return spdlog::level::level_enum::err;
            case crow::LogLevel::Critical:
                return spdlog::level::level_enum::critical;
            }
            // Unhandled log levels are an error.
            return spdlog::level::err;
        }();

        // Do the thing.
        spdlog::log(level, message);
    }
} // namespace sacnlogger
