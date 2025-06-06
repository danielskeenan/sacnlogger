/**
 * @file main.cpp
 *
 * @author Dan Keenan
 * @date 3/1/25
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
#include <csignal>
#include <etcpal/common.h>
#include <sacn/cpp/common.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <thread>

#include "EtcPalLogHandler.h"
#include "sacnlogger_config.h"
#include "sacnloggerlib/Config.h"
#include "sacnloggerlib/ConfigException.h"
#include "sacnloggerlib/Runner.h"
#include "sacnloggerlib/UniverseMonitor.h"

bool termRequested = false;

void requestTerm(int) { termRequested = true; }

void sacnCleanup()
{
    sacn::Deinit();
    etcpal_deinit(ETCPAL_FEATURE_LOGGING);
    spdlog::shutdown();
}

int main(int argc, char* argv[])
{
    argparse::ArgumentParser parser(sacnlogger::config::kProjectName, sacnlogger::config::kProjectVersion);
    parser.add_description(sacnlogger::config::kProjectDescription);
    parser.add_argument("config").help("path to configuration file");
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

    // Setup logger.
    auto logger = spdlog::stdout_color_mt("app");
    spdlog::set_default_logger(logger);
    spdlog::default_logger()->sinks().emplace_back(new spdlog::sinks::rotating_file_sink_mt("app.log", 5242880, 99));
    SPDLOG_INFO("Starting sACN logger");

    // Setup EtcPal.
    etcpal_init(ETCPAL_FEATURE_LOGGING);
    sacnlogger::EtcPalLogHandler etcpalLogHandler;
    std::unique_ptr<etcpal::Logger, sacnlogger::EtcPalLoggerDeleter> etcpalLogger(new etcpal::Logger);
    etcpalLogger->SetSyslogAppName(sacnlogger::config::kProjectName).Startup(etcpalLogHandler);

    // Setup sACN.
    sacn::Init(*etcpalLogger);
    std::atexit(sacnCleanup);

    // Setup signal handling.
    std::signal(SIGTERM, &requestTerm);
    std::signal(SIGINT, &requestTerm);
#ifdef SIGQUIT
    std::signal(SIGQUIT, &requestTerm);
#endif

    // Load config.
    sacnlogger::Config config;
    try
    {
        config = sacnlogger::Config::loadFromFile(parser.get<std::string>("config"));
    }
    catch (const sacnlogger::ConfigException& e)
    {
        return EXIT_FAILURE;
    }

    sacnlogger::Runner runner(config);
    runner.start();

    auto waiter = std::thread(
        []()
        {
            while (!termRequested)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    waiter.join();

    SPDLOG_INFO("Stopping sACN logger");
    runner.stop();

    return EXIT_SUCCESS;
}
