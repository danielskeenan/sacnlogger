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

#include <boost/dll/runtime_symbol_info.hpp>
#include <etcpal/cpp/netint.h>
#include <inja/inja.hpp>
#include <sacnlogger_share.h>
#include <spdlog/spdlog.h>
#include "sacnlogger_config.h"

namespace sacnlogger
{
    namespace detail
    {
        void to_json(nlohmann::json& j, const NetworkConfig& value)
        {
            j = nlohmann::json{{"dhcp", value.dhcp},
                               {"address", value.address.ToString()},
                               {"mask", value.mask.ToString()},
                               {"prefixLength", value.mask.MaskLength()},
                               {"gateway", value.gateway.ToString()},
                               {"ntp", value.ntp},
                               {"ntpServer", value.ntpServer.toString()}};
        }

        void from_json(const nlohmann::json& j, NetworkConfig& value)
        {
            j.at("dhcp").get_to(value.dhcp);
            value.address = etcpal::IpAddr::FromString(j.at("address"));
            value.mask = etcpal::IpAddr::FromString(j.at("mask"));
            value.gateway = etcpal::IpAddr::FromString(j.at("gateway"));
            j.at("ntp").get_to(value.ntp);
            value.ntpServer = AddressOrHostname(j.at("ntpServer"));
        }
    } // namespace detail

    void SystemConfig::readFromSystem()
    {
        // Network.
        detail::NetworkConfig newNetworkConfig;
        const auto ifaceIx = etcpal::netint::GetDefaultInterface(etcpal::IpAddrType::kV4);
        if (ifaceIx.has_value())
        {
            const auto ifaces = etcpal::netint::GetInterfacesForIndex(ifaceIx.value());
            if (ifaces.has_value())
            {
                newNetworkConfig.address = ifaces->front().addr();
                newNetworkConfig.mask = ifaces->front().mask();
                // TODO: Gateway
            }
        }
        networkConfig = newNetworkConfig;
    }

    void SystemConfig::writeToSystem()
    {
        inja::Environment env{templateDir(), ""};

        // Network.
        SPDLOG_INFO("Writing new network config to {}", netConfigFile().string());
        env.write("01-eth0.network", nlohmann::json(*this), netConfigFile());
    }

    std::filesystem::path SystemConfig::templateDir() { return config::shareDir() / "system_config" / "templates/"; }

    std::filesystem::path SystemConfig::systemPrefix()
    {
        static std::filesystem::path resolvedPrefix;
        if (resolvedPrefix.empty())
        {
            auto sysPrefix = std::getenv("SACNLOGGER_SYS_PREFIX");
            if (sysPrefix == nullptr || sysPrefix == "")
            {
                resolvedPrefix = "/";
            }
            else
            {
                resolvedPrefix = sysPrefix;
            }
            resolvedPrefix = std::filesystem::absolute(resolvedPrefix);
        }
        return resolvedPrefix;
    }

    std::filesystem::path SystemConfig::netConfigFile()
    {
        const auto path = systemPrefix() / "etc" / "systemd" / "network" / "01-eth0.network";
        std::filesystem::create_directories(path.parent_path());
        return path;
    }

    void to_json(nlohmann::json& j, const SystemConfig& value) { j = nlohmann::json{{"network", value.networkConfig}}; }

    void from_json(const nlohmann::json& j, SystemConfig& value) { j.get_to(value.networkConfig); }

} // namespace sacnlogger
