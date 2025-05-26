/**
 * @file DiskSpaceMonitor.cpp
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

#include "sacnloggerlib/DiskSpaceMonitor.h"
#include <spdlog/spdlog.h>

namespace sacnlogger
{
    DiskSpaceMonitor::DiskSpaceMonitor(const std::filesystem::path& path) : path_(path)
    {
        worker_ = std::jthread(
            [this](std::stop_token stop)
            {
                while (!stop.stop_requested())
                {
                    if (!path_.empty())
                    {
                        std::scoped_lock lock(configMx_);
                        try
                        {
                            // Check space.
                            const auto space = std::filesystem::space(path_);
                            if (space.available <= criticalSpaceThreshold_)
                            {
                                if (!criticalSpaceMet_)
                                {
                                    sigCriticalSpace(space.available);
                                    criticalSpaceMet_ = true;
                                }
                            }
                            else if (space.available <= lowSpaceThreshold_)
                            {
                                if (!lowSpaceMet_)
                                {
                                    sigLowSpace(space.available);
                                    lowSpaceMet_ = true;
                                }
                            }
                            else
                            {
                                criticalSpaceMet_ = false;
                                lowSpaceMet_ = false;
                            }
                        }
                        catch (const std::exception& e)
                        {
                            SPDLOG_WARN("DiskSpaceMonitor: {}", e.what());
                        }

                        // Wait for the next poll period, checking for stops more frequently.
                        const auto continueAt = std::chrono::steady_clock::now() + pollPeriod_;
                        while (std::chrono::steady_clock::now() < continueAt)
                        {
                            if (stop.stop_requested())
                            {
                                break;
                            }
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                        }
                    }
                }
            });
    }

} // namespace sacnlogger
