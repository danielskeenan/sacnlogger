/**
 * @file NetworkConfig.h
 *
 * @author Dan Keenan
 * @date 5/18/25
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

#ifndef NETWORKCONFIG_H
#define NETWORKCONFIG_H

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <etcpal/cpp/inet.h>
#include <filesystem>
#include <nlohmann/json_fwd.hpp>
#include <sdbus-c++/IConnection.h>
#include "AddressOrHostname.h"

namespace sacnlogger::detail
{
    class NetworkConfig
    {
        friend bool operator==(const NetworkConfig& lhs, const NetworkConfig& rhs)
        {
            return std::tie(lhs.dhcp, lhs.address, lhs.mask, lhs.gateway, lhs.ntp, lhs.ntpServer) ==
                std::tie(rhs.dhcp, rhs.address, rhs.mask, rhs.gateway, rhs.ntp, rhs.ntpServer);
        }

    public:
        explicit NetworkConfig(const std::shared_ptr<sdbus::IConnection>& dbus = {}) : dbus_(dbus) {}

        bool dhcp = true;
        etcpal::IpAddr address;
        etcpal::IpAddr mask;
        etcpal::IpAddr gateway;
        bool ntp = true;
        AddressOrHostname ntpServer;

        void readFromSystem();
        void writeToSystem();

    private:
        std::shared_ptr<sdbus::IConnection> dbus_;

        /** Path to systemd-network interface configuration file relative to systemPrefix(). */
        static std::filesystem::path netConfigFile();
        /** Convert an IP Address in array format as returned by networkd's Describe() methods. */
        static const boost::bimap<boost::bimaps::unordered_set_of<int>,
                                  boost::bimaps::unordered_set_of<etcpal::IpAddrType>>
            kNetworkdAddressFamilies;
        static etcpal::IpAddr ipAddrFromNetworkdJson(const nlohmann::json& family, const nlohmann::json& address);
    };

    void to_json(nlohmann::json& j, const NetworkConfig& value);
    void from_json(const nlohmann::json& j, NetworkConfig& value);
} // namespace sacnlogger::detail

#endif // NETWORKCONFIG_H
