/**
 * @file NetworkConfig.cpp
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

#include "sacnloggerlib/SystemConfig/NetworkConfig.h"

#include <boost/endian/arithmetic.hpp>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <sacnlogger_share.h>
#include <sdbus-c++/sdbus-c++.h>
#include <spdlog/spdlog.h>

namespace sacnlogger::detail
{
    decltype(NetworkConfig::kNetworkdAddressFamilies) NetworkConfig::kNetworkdAddressFamilies = []()
    {
        std::remove_const_t<decltype(NetworkConfig::kNetworkdAddressFamilies)> o;
        o.insert({2, etcpal::IpAddrType::kV4});
        o.insert({10, etcpal::IpAddrType::kV6});

        return o;
    }();
    void NetworkConfig::readFromSystem()
    {
        NetworkConfig newNetworkConfig(dbus_);
        const sdbus::ServiceName networkdService{"org.freedesktop.network1"};

        // Find eth0 object path for later use.
        sdbus::ObjectPath eth0ObjectPath;
        {
            std::vector<sdbus::Struct<int32_t, std::string, sdbus::ObjectPath>> ifaces;
            auto networkdManager =
                sdbus::createProxy(*dbus_, networkdService, sdbus::ObjectPath{"/org/freedesktop/network1"});
            networkdManager->callMethod("ListLinks")
                .onInterface("org.freedesktop.network1.Manager")
                .storeResultsTo(ifaces);
            for (const auto& iface : ifaces)
            {
                if (iface.get<1>() == "eth0")
                {
                    eth0ObjectPath = iface.get<2>();
                }
                break;
            }
        }
        if (eth0ObjectPath.empty())
        {
            SPDLOG_ERROR("networkd doesn't know about eth0.");
            *this = newNetworkConfig;
            return;
        }

        // Get info about eth0.
        nlohmann::json eth0Info;
        {
            std::string eth0Json;
            auto eth0Link = sdbus::createProxy(*dbus_, networkdService, eth0ObjectPath);
            eth0Link->callMethod("Describe").onInterface("org.freedesktop.network1.Link").storeResultsTo(eth0Json);
            eth0Info = nlohmann::json::parse(eth0Json);
        }

        // Address.
        const auto addresses = eth0Info.find("Addresses");
        if (addresses != eth0Info.end())
        {
            // Prefer IPv4 before IPv6.
            for (const auto [addressFamily, ipAddrType] : kNetworkdAddressFamilies.left)
            {
                const auto familyAddress =
                    std::ranges::find_if(addresses.value(), [addressFamily](const nlohmann::json& j)
                                         { return j["Family"] == addressFamily; });
                if (familyAddress != addresses.value().end())
                {
                    newNetworkConfig.dhcp = familyAddress.value()["ConfigSource"] != "static";
                    newNetworkConfig.address = ipAddrFromNetworkdJson(addressFamily, familyAddress.value()["Address"]);
                    const auto prefixLength = familyAddress.value()["PrefixLength"].get<unsigned int>();
                    newNetworkConfig.mask = etcpal::IpAddr::Netmask(ipAddrType, prefixLength);
                    break;
                }
            }
        }

        // Gateway
        const auto routes = eth0Info.find("Routes");
        if (routes != eth0Info.end() && newNetworkConfig.address.IsValid())
        {
            const auto addressFamily = kNetworkdAddressFamilies.right.at(newNetworkConfig.address.type());
            const auto defaultRoute =
                std::ranges::find_if(routes.value(),
                                     [addressFamily](const nlohmann::json& j)
                                     {
                                         return j["Family"] == addressFamily &&
                                             j["DestinationPrefixLength"].get<unsigned int>() == 0 &&
                                             j.find("Gateway") != j.end();
                                     });
            if (defaultRoute != routes.value().end())
            {
                newNetworkConfig.gateway = ipAddrFromNetworkdJson(addressFamily, defaultRoute.value()["Gateway"]);
            }
        }

        // NTP Config.
        const auto ntp = eth0Info.find("NTP");
        if (ntp != eth0Info.end() && !ntp.value().empty())
        {
            const auto ntpInfo = ntp.value().at(0);
            newNetworkConfig.ntp = true;
            const auto address = ntpInfo.find("Address");
            if (address != ntpInfo.end())
            {
                newNetworkConfig.ntpServer = ipAddrFromNetworkdJson(ntpInfo["Family"], address.value());
            }
            const auto server = ntpInfo.find("Server");
            if (server != ntpInfo.end())
            {
                // Hostname.
                newNetworkConfig.ntpServer = server.value();
            }
        }
        else
        {
            newNetworkConfig.ntp = false;
            newNetworkConfig.ntpServer = AddressOrHostname{};
        }

        *this = newNetworkConfig;
    }

    void NetworkConfig::writeToSystem()
    {
        inja::Environment env{config::templateDir(), ""};

        // Network.
        SPDLOG_INFO("Writing new network config to {}", netConfigFile().string());
        nlohmann::json j(*this);
        j["prefixLength"] = mask.MaskLength();
        env.write("01-eth0.network", j, netConfigFile());
    }

    std::filesystem::path NetworkConfig::netConfigFile()
    {
        const auto path = config::systemPrefix() / "etc" / "systemd" / "network" / "01-eth0.network";
        std::filesystem::create_directories(path.parent_path());
        return path;
    }

    etcpal::IpAddr NetworkConfig::ipAddrFromNetworkdJson(const nlohmann::json& family, const nlohmann::json& address)
    {
        if (family == kNetworkdAddressFamilies.right.at(etcpal::IpAddrType::kV4))
        {
            // IPv4.
            // etcpal::IpAddr requires the data be in host order.
            boost::endian::big_uint32_t addressVal{
                static_cast<unsigned int>(address[0].get<uint8_t>() << 24 | address[1].get<uint8_t>() << 16 |
                                          address[2].get<uint8_t>() << 8 | address[3].get<uint8_t>())};
            return {addressVal};
        }

        if (family == kNetworkdAddressFamilies.right.at(etcpal::IpAddrType::kV4))
        {
            // IPv6.
            std::array<uint8_t, ETCPAL_IPV6_BYTES> addressVal{};
            address.get_to(addressVal);
            return etcpal::IpAddr(addressVal.data());
        }

        return {};
    }

    void to_json(nlohmann::json& j, const NetworkConfig& value)
    {
        j = nlohmann::json{{"dhcp", value.dhcp},
                           {"address", value.address.ToString()},
                           {"mask", value.mask.ToString()},
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
} // namespace sacnlogger::detail
