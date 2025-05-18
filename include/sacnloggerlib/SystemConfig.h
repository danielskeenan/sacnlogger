/**
 * @file SystemConfig.h
 *
 * @author Dan Keenan
 * @date 5/3/25
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

#ifndef SYSTEMCONFIG_H
#define SYSTEMCONFIG_H

#include <filesystem>
#include "SystemConfig/NetworkConfig.h"

namespace sdbus
{
    class IConnection;
}

namespace sacnlogger
{
    /**
     * Read/Write system hardware configuration.
     */
    class SystemConfig
    {
        friend bool operator==(const SystemConfig& lhs, const SystemConfig& rhs)
        {
            return lhs.networkConfig == rhs.networkConfig;
        }

    public:
        /**
         * @param dbus A custom dbus connection to use (usually for testing). Defaults to the system bus.
         */
        explicit SystemConfig(const std::shared_ptr<sdbus::IConnection>& dbus);
        SystemConfig() : SystemConfig(nullptr) {}

        detail::NetworkConfig networkConfig;

        void readFromSystem();
        void writeToSystem();

    private:
        // Allow testing with a fake dbus connection.
        std::shared_ptr<sdbus::IConnection> dbus_;
    };

    void to_json(nlohmann::json& j, const SystemConfig& value);
    void from_json(const nlohmann::json& j, SystemConfig& value);
} // namespace sacnlogger

#endif // SYSTEMCONFIG_H
