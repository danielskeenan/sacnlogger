/**
 * @file main.cpp
 *
 * @author Dan Keenan
 * @date 4/21/25
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

#include <filesystem>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "argparse/argparse.hpp"
#include "sacnlogger_config.h"

static constexpr auto kDiskDevice = "/dev/sacnlogger-disk";
static constexpr auto kDiskDir = "/media/sacnlogger/disk";
static constexpr auto kConfigFile = "config.yaml";

bool diskAvailable()
{
    spdlog::debug("Checking if {} is available.", kDiskDevice);
    if (!std::filesystem::is_symlink(kDiskDevice))
    {
        spdlog::debug("{} not available.", kDiskDevice);
        return false;
    }
    const auto diskDev = std::filesystem::path(kDiskDevice).parent_path() / std::filesystem::read_symlink(kDiskDevice);
    spdlog::debug("Checking if {} is a block file.", diskDev.string());
    if (!std::filesystem::is_block_file(diskDev))
    {
        spdlog::debug("{} not a block file.", diskDev.string());
        return false;
    }
    spdlog::debug("Checking if {} mounted.", kDiskDir);
    if (!std::filesystem::is_directory(kDiskDir))
    {
        spdlog::debug("{} not a directory.", kDiskDir);
        return false;
    }
    const auto configPath = std::filesystem::path(kDiskDir) / kConfigFile;
    spdlog::debug("Checking for config file '{}'.", configPath.string());
    if (!std::filesystem::is_regular_file(configPath))
    {
        spdlog::debug("{} not a file.", configPath.string());
    }
    // TODO: Validate config file.
    return true;
}

int main(int argc, char* argv[])
{
    // Arguments
    argparse::ArgumentParser parser(fmt::format("{} (Monitor)", sacnlogger::config::kProjectName),
                                    sacnlogger::config::kProjectVersion);
    parser.add_description(sacnlogger::config::kProjectDescription);
    auto logArg = parser.add_argument("--log")
                      .help("log level {trace, debug, info, warning, error, critical, off}")
                      .choices("trace", "debug", "info", "warning", "error", "critical", "off");
    parser.add_argument("--exe").help("path to sacnlogger executable").default_value("sacnlogger");
    try
    {
        parser.parse_args(argc, argv);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return EXIT_FAILURE;
    }

    // Set log level.
    if (parser.is_used("log"))
    {
        const auto loglevelName = parser.get<std::string>("log");
        const auto spdlogLevel = [&loglevelName]() -> spdlog::level::level_enum
        {
            if (loglevelName == "trace")
                return spdlog::level::trace;
            else if (loglevelName == "debug")
                return spdlog::level::debug;
            else if (loglevelName == "info")
                return spdlog::level::info;
            else if (loglevelName == "warning")
                return spdlog::level::warn;
            else if (loglevelName == "error")
                return spdlog::level::err;
            else if (loglevelName == "critical")
                return spdlog::level::critical;
            else if (loglevelName == "off")
                return spdlog::level::off;
            throw std::logic_error("Unknown log level '" + loglevelName + "'");
        }();
        spdlog::set_level(spdlogLevel);
    }

    // Wait for disk to be available.
    while (!diskAvailable())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::filesystem::current_path(kDiskDir);
    const auto configPath = std::filesystem::path(kDiskDir) / kConfigFile;

    // Run program.
    const auto exePath = parser.get<std::string>("exe");
    if (!std::filesystem::is_regular_file(exePath))
    {
        spdlog::critical("Executable {} does not exist.", exePath);
        return EXIT_FAILURE;
    }
    std::system(fmt::format("{} {}", exePath, configPath.string()).c_str());

    return EXIT_SUCCESS;
}
