/**
 * @file SystemConfig.cpp
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

#include "sacnloggerlib/SystemConfig.h"
#include <nlohmann/json.hpp>
#include <sacnlogger_share.h>
#include <sdbus-c++/sdbus-c++.h>

namespace sacnlogger
{
    SystemConfig::SystemConfig(const std::shared_ptr<sdbus::IConnection>& dbus)
    {
        if (!dbus)
        {
            // Deleted in destructor.
            dbus_ = sdbus::createSystemBusConnection();
        }
        else
        {
            dbus_ = dbus;
        }
        networkConfig = detail::NetworkConfig(dbus_);
    }

    void SystemConfig::readFromSystem() { networkConfig.readFromSystem(); }

    void SystemConfig::writeToSystem() { networkConfig.writeToSystem(); }

    void to_json(nlohmann::json& j, const SystemConfig& value) { j = nlohmann::json{{"network", value.networkConfig}}; }

    void from_json(const nlohmann::json& j, SystemConfig& value) { j.get_to(value.networkConfig); }

} // namespace sacnlogger
