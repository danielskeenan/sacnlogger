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
#include <spdlog/spdlog.h>
#include <thread>

#include "EtcPalLogHandler.h"
#include "sacnlogger_config.h"
#include "sacnloggerlib/Config.h"
#include "sacnloggerlib/ConfigException.h"
#include "sacnloggerlib/UniverseMonitor.h"

bool termRequested = false;
std::vector<sacnlogger::UniverseMonitor> universeMonitors;

void sigTerm(int) { termRequested = true; }

void sacnCleanup()
{
    universeMonitors.clear();
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

    // Setup EtcPal.
    etcpal_init(ETCPAL_FEATURE_LOGGING);
    sacnlogger::EtcPalLogHandler etcpalLogHandler;
    std::unique_ptr<etcpal::Logger, sacnlogger::EtcPalLoggerDeleter> etcpalLogger(new etcpal::Logger);
    etcpalLogger->SetSyslogAppName(sacnlogger::config::kProjectName).Startup(etcpalLogHandler);

    // Setup sACN.
    sacn::Init(*etcpalLogger);
    std::atexit(sacnCleanup);

    // Setup signal handling.
    std::signal(SIGTERM, &sigTerm);

    // Load config.
    sacnlogger::Config config;
    try
    {
        config = sacnlogger::Config::loadFromFile(parser.get<std::string>("config"));
    }
    catch (const sacnlogger::ConfigException& e)
    {
        std::cerr << "Correct the error and run again." << std::endl;
        return EXIT_FAILURE;
    }

    // Create monitors.
    for (const auto universe : config.universes)
    {
        auto& universeMonitor = universeMonitors.emplace_back(universe);
        universeMonitor.setUsePap(config.usePap);
        universeMonitor.start();
    }

    auto waiter = std::thread(
        []()
        {
            while (!termRequested)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    waiter.join();

    return EXIT_SUCCESS;
}
