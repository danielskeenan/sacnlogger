/**
 * @file Runner.cpp
 *
 * @author Dan Keenan
 * @date 5/24/25
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

#include "sacnloggerlib/Runner.h"
#include <spdlog/spdlog.h>
#include <fmt/ranges.h>

namespace sacnlogger
{
    void Runner::start()
    {
#ifdef SACNLOGGER_EMBEDDED_BUILD
        // Configure system.
        if (config_.systemConfig.networkConfig.dhcp)
        {
            SPDLOG_INFO("Network: DHCP");
        }
        else
        {
            SPDLOG_INFO("Network: {} / {} / {}", config_.systemConfig.networkConfig.address.ToString(),
                        config_.systemConfig.networkConfig.mask.ToString(),
                        config_.systemConfig.networkConfig.gateway.ToString());
        }
        SPDLOG_INFO("NTP: {}", config_.systemConfig.networkConfig.ntp ? "enabled" : "disabled");
        if (config_.systemConfig.networkConfig.ntpServer.isValid())
        {
            SPDLOG_INFO("Static NTP Server: {}", config_.systemConfig.networkConfig.ntpServer.toString());
        }
        config_.systemConfig.writeToSystem();
#endif

        SPDLOG_INFO("Using universes {}", config_.universes);
        SPDLOG_INFO("PAP = {}", config_.usePap);

        // Create monitors.
        for (const auto universe : config_.universes)
        {
            auto& universeMonitor = universeMonitors_.emplace_back(universe);
            universeMonitor.setUsePap(config_.usePap);
            universeMonitor.start();
        }
        running_ = true;
    }

    void Runner::stop()
    {
        universeMonitors_.clear();
        running_ = false;
    }

    void Runner::setConfig(const Config& config)
    {
        const auto wasRunning = running_;
        if (wasRunning)
        {
            stop();
        }

        config_ = config;

        if (wasRunning)
        {
            start();
        }
    }

} // namespace sacnlogger
