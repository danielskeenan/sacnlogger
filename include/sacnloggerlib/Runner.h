/**
 * @file Runner.h
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

#ifndef RUNNER_H
#define RUNNER_H

#include <future>
#include <vector>
#include "Config.h"
#include "UniverseMonitor.h"

namespace sacnlogger
{
    /**
     * Main application runner.
     */
    class Runner
    {
    public:
        explicit Runner(const Config& config = {}) : config_(config) {}

        /**
         * Start the application (blocking).
         */
        void start();

        /**
         * Start the application (non-blocking).
         * @return Store this future, otherwise it will be started from the calling thread.
         */
        std::future<void> startAsync()
        {
            return std::async(std::launch::async, [&]() { this->start(); });
        }

        /**
         * Stop the application.
         */
        void stop();

        [[nodiscard]] const Config& config() const { return config_; }

        /**
         * Set a new config, restarting the application if it was running.
         * @param config
         */
        void setConfig(const Config& config);

    private:
        Config config_;
        bool running_ = false;
        std::vector<UniverseMonitor> universeMonitors_;
    };
} // namespace sacnlogger


#endif // RUNNER_H
