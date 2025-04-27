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

#include <argparse/argparse.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/process/v2/process.hpp>
#include <boost/process/v2/environment.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <thread>
#include "sacnlogger_config.h"

static constexpr auto kDiskDir = "/media/sacnlogger/disk";
static constexpr auto kConfigFile = "config.yaml";

bool termRequested = false;

bool diskAvailable()
{
    SPDLOG_DEBUG("Checking if {} mounted.", kDiskDir);
    if (!std::filesystem::is_directory(kDiskDir))
    {
        SPDLOG_DEBUG("{} not a directory.", kDiskDir);
        return false;
    }
    const auto configPath = std::filesystem::path(kDiskDir) / kConfigFile;
    SPDLOG_DEBUG("Checking for config file '{}'.", configPath.string());
    if (!std::filesystem::is_regular_file(configPath))
    {
        SPDLOG_DEBUG("{} not a file.", configPath.string());
        return false;
    }
    SPDLOG_DEBUG("Checking if config file '{}' is readable.");
    std::ifstream configFile(configPath.string());
    if (!configFile.is_open())
    {
        SPDLOG_DEBUG("{} not readable.", configPath.string());
        return false;
    }
    // TODO: Validate config file.
    return true;
}

void requestTerm(int) { termRequested = true; }

int main(int argc, char* argv[])
{
    // Arguments
    argparse::ArgumentParser parser(fmt::format("{} (Monitor)", sacnlogger::config::kProjectName),
                                    sacnlogger::config::kProjectVersion);
    parser.add_description(sacnlogger::config::kProjectDescription);
    auto logArg = parser.add_argument("--log")
                      .help("log level {trace, debug, info, warning, error, critical, off}")
                      .choices("trace", "debug", "info", "warning", "error", "critical", "off");
    parser.add_argument("--exe").help("path to sacnlogger executable");
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

    // Setup logging.
    auto logger = spdlog::stdout_color_mt("monitor");
    spdlog::set_default_logger(logger);
    if (parser.is_used("log"))
    {
        const auto loglevelName = parser.get<std::string>("log");
        const auto spdlogLevel = spdlog::level::from_str(loglevelName);
        spdlog::set_level(spdlogLevel);
    }

    // Setup signal handling.
    std::signal(SIGTERM, &requestTerm);
    std::signal(SIGINT, &requestTerm);
#ifdef SIGQUIT
    std::signal(SIGQUIT, &requestTerm);
#endif

    // Wait for disk to be available.
    while (!diskAvailable())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // Add monitor log on disk.
    spdlog::default_logger()->sinks().emplace_back(
        new spdlog::sinks::rotating_file_sink_mt("monitor.log", 5242880, 99));
    std::filesystem::current_path(kDiskDir);
    const auto configPath = std::filesystem::path(kDiskDir) / kConfigFile;

    // Run program.
    boost::filesystem::path exePath;
    if (parser.is_used("exe"))
    {
        exePath = parser.get<std::string>("exe");
    }
    else
    {
        exePath = boost::process::v2::environment::find_executable("sacnlogger");
    }
    if (!boost::filesystem::is_regular_file(exePath))
    {
        SPDLOG_CRITICAL("Executable {} does not exist.", exePath.string());
        return EXIT_FAILURE;
    }
    SPDLOG_INFO("Starting executable");
    boost::asio::io_context ioCtx;
    boost::process::v2::process exe(ioCtx, exePath, {configPath.string()});
    auto waiter = std::thread(
        [&exe]()
        {
            while (!termRequested && exe.running())
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    waiter.join();

    SPDLOG_INFO("Stopping executable");
    if (exe.running())
    {
        exe.request_exit();
        exe.wait();
    }
    spdlog::shutdown();

    return exe.exit_code();
}
