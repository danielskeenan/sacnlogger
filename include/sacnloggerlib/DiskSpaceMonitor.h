/**
 * @file DiskSpaceMonitor.h
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

#ifndef DISKSPACEMONITOR_H
#define DISKSPACEMONITOR_H

#include <boost/signals2/signal.hpp>
#include <filesystem>
#include <mutex>
#include <thread>

namespace sacnlogger
{

    /**
     * Monitor a path for available space.
     */
    class DiskSpaceMonitor
    {
    public:
        explicit DiskSpaceMonitor(const std::filesystem::path& path = {});

        [[nodiscard]] const std::filesystem::path& path() const { return path_; }
        void setPath(const std::filesystem::path& path)
        {
            std::scoped_lock lock(configMx_);
            path_ = path;
        }
        [[nodiscard]] const std::uintmax_t& lowSpaceThreshold() const { return lowSpaceThreshold_; }
        void setLowSpaceThreshold(const std::uintmax_t& lowSpaceThreshold)
        {
            std::scoped_lock lock(configMx_);
            lowSpaceThreshold_ = lowSpaceThreshold;
        }
        [[nodiscard]] const std::uintmax_t& criticalSpaceThreshold() const { return criticalSpaceThreshold_; }
        void setCriticalSpaceThreshold(const std::uintmax_t& criticalSpaceThreshold)
        {
            std::scoped_lock lock(configMx_);
            criticalSpaceThreshold_ = criticalSpaceThreshold;
        }
        [[nodiscard]] const std::chrono::seconds& pollPeriod() const { return pollPeriod_; }
        void setPollPeriod(const std::chrono::seconds& pollPeriod)
        {
            std::scoped_lock lock(configMx_);
            pollPeriod_ = pollPeriod;
        }

        using SigLowSpace = boost::signals2::signal<void(std::uintmax_t)>;
        /**
         * Emitted when available space is between lowSpaceThreshold() and criticalSpaceThreshold().
         */
        SigLowSpace sigLowSpace;

        using SigCriticalSpace = boost::signals2::signal<void(std::uintmax_t)>;
        /**
         * Emitted when available space is below criticalSpaceThreshold().
         */
        SigCriticalSpace sigCriticalSpace;

    private:
        std::mutex configMx_;
        std::filesystem::path path_;
        std::uintmax_t lowSpaceThreshold_{1073741824}; // 1GiB
        std::atomic<bool> lowSpaceMet_{false};
        std::uintmax_t criticalSpaceThreshold_{104857600}; // 100MiB;
        std::atomic<bool> criticalSpaceMet_{false};
        std::chrono::seconds pollPeriod_{10};
        std::jthread worker_;
    };

} // namespace sacnlogger

#endif // DISKSPACEMONITOR_H
