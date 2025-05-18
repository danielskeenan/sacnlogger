/**
 * @file SystemConfig.cpp
 *
 * @author Dan Keenan
 * @date 5/4/25
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

#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <sacnloggerlib/SystemConfig.h>
#include <thread>
#include "FakeDbus.h"
#include "FileMatcher.h"

class ReturnFileContents
{
public:
    explicit ReturnFileContents(const std::string& path) : path_(path) {}

    std::string operator()() const
    {
        std::ifstream file(path_);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file");
        }
        return {std::istreambuf_iterator(file), {}};
    }

private:
    std::string path_;
};

TEST_CASE("Network Write")
{
    FakeDbus dbus;
    const sdbus::ServiceName networkdService{"org.freedesktop.network1"};
    bool reloaded = false;
    auto networkdManager = sdbus::createObject(*dbus.server, sdbus::ObjectPath{"/org/freedesktop/network1"});
    auto reload = [&reloaded]() { reloaded = true; };
    networkdManager->addVTable(sdbus::registerMethod("Reload").implementedAs(std::move(reload)))
        .forInterface("org.freedesktop.network1.Manager");

    sacnlogger::SystemConfig systemConfig(dbus.client);

    SECTION("DHCP")
    {
        systemConfig.networkConfig.dhcp = true;

        SECTION("Without NTP")
        {
            systemConfig.networkConfig.ntp = false;
            REQUIRE(!reloaded);
            systemConfig.writeToSystem();
            CHECK_THAT(RESOURCES_PATH "/SystemConfigTest/network/dhcp.network",
                       EqualsFile(SACNLOGGER_SYS_PREFIX "/etc/systemd/network/01-eth0.network"));
            CHECK(reloaded);
        }
        SECTION("With NTP from DHCP")
        {
            systemConfig.networkConfig.ntp = true;
            REQUIRE(!reloaded);
            systemConfig.writeToSystem();
            CHECK_THAT(RESOURCES_PATH "/SystemConfigTest/network/dhcp-ntp.network",
                       EqualsFile(SACNLOGGER_SYS_PREFIX "/etc/systemd/network/01-eth0.network"));
            CHECK(reloaded);
        }
        SECTION("With NTP from static")
        {
            systemConfig.networkConfig.ntp = true;
            systemConfig.networkConfig.ntpServer = "us.pool.ntp.org";
            REQUIRE(!reloaded);
            systemConfig.writeToSystem();
            CHECK_THAT(RESOURCES_PATH "/SystemConfigTest/network/dhcp-ntpstatic.network",
                       EqualsFile(SACNLOGGER_SYS_PREFIX "/etc/systemd/network/01-eth0.network"));
            CHECK(reloaded);
        }
    }

    SECTION("Static")
    {
        systemConfig.networkConfig.dhcp = false;
        systemConfig.networkConfig.address = etcpal::IpAddr::FromString("192.168.2.101");
        systemConfig.networkConfig.mask = etcpal::IpAddr::FromString("255.255.255.0");
        systemConfig.networkConfig.gateway = etcpal::IpAddr::FromString("192.168.2.1");
        SECTION("Without NTP")
        {
            systemConfig.networkConfig.ntp = false;
            REQUIRE(!reloaded);
            systemConfig.writeToSystem();
            CHECK_THAT(RESOURCES_PATH "/SystemConfigTest/network/static.network",
                       EqualsFile(SACNLOGGER_SYS_PREFIX "/etc/systemd/network/01-eth0.network"));
            CHECK(reloaded);
        }
        SECTION("With NTP")
        {
            systemConfig.networkConfig.ntp = true;
            systemConfig.networkConfig.ntpServer = "us.pool.ntp.org";
            REQUIRE(!reloaded);
            systemConfig.writeToSystem();
            CHECK_THAT(RESOURCES_PATH "/SystemConfigTest/network/static-ntp.network",
                       EqualsFile(SACNLOGGER_SYS_PREFIX "/etc/systemd/network/01-eth0.network"));
            CHECK(reloaded);
        }
    }
}

TEST_CASE("Network Read")
{
    FakeDbus dbus;

    // Mock requests for interfaces.
    const sdbus::ServiceName networkdService{"org.freedesktop.network1"};
    const sdbus::InterfaceName linkInterface{"org.freedesktop.network1.Link"};
    const sdbus::MethodName describeMethod{"Describe"};
    const sdbus::ObjectPath loObjectPath{"/org/freedesktop/network1/link/_31"};
    const sdbus::ObjectPath eth0ObjectPath{"/org/freedesktop/network1/link/_32"};
    auto networkdManager = sdbus::createObject(*dbus.server, sdbus::ObjectPath{"/org/freedesktop/network1"});
    auto listLinks = [&]()
    {
        return std::vector<sdbus::Struct<int32_t, std::string, sdbus::ObjectPath>>{
            {2, "eth0", eth0ObjectPath},
            {1, "lo", loObjectPath},
        };
    };
    networkdManager->addVTable(sdbus::registerMethod("ListLinks").implementedAs(std::move(listLinks)))
        .forInterface("org.freedesktop.network1.Manager");
    auto loObject = sdbus::createObject(*dbus.server, loObjectPath);
    loObject
        ->addVTable(sdbus::registerMethod(describeMethod)
                        .implementedAs(ReturnFileContents(RESOURCES_PATH "/SystemConfigTest/network/lo.json")))
        .forInterface(linkInterface);
    auto eth0Object = sdbus::createObject(*dbus.server, eth0ObjectPath);

    sacnlogger::SystemConfig systemConfig(dbus.client);
    SECTION("DHCP")
    {
        SECTION("Without NTP")
        {
            eth0Object
                ->addVTable(
                    sdbus::registerMethod(describeMethod)
                        .implementedAs(ReturnFileContents(RESOURCES_PATH "/SystemConfigTest/network/dhcp.json")))
                .forInterface(linkInterface);

            systemConfig.readFromSystem();
            CHECK(systemConfig.networkConfig.ntp == false);
            CHECK(systemConfig.networkConfig.ntpServer == sacnlogger::AddressOrHostname{});
        }
        SECTION("With NTP from DHCP")
        {
            eth0Object
                ->addVTable(
                    sdbus::registerMethod(describeMethod)
                        .implementedAs(ReturnFileContents(RESOURCES_PATH "/SystemConfigTest/network/dhcp-ntp.json")))
                .forInterface(linkInterface);

            systemConfig.readFromSystem();
            CHECK(systemConfig.networkConfig.ntp == true);
            CHECK(systemConfig.networkConfig.ntpServer == sacnlogger::AddressOrHostname("192.168.3.101"));
        }
        SECTION("With NTP from static")
        {
            eth0Object
                ->addVTable(sdbus::registerMethod(describeMethod)
                                .implementedAs(
                                    ReturnFileContents(RESOURCES_PATH "/SystemConfigTest/network/dhcp-ntpstatic.json")))
                .forInterface(linkInterface);

            systemConfig.readFromSystem();
            CHECK(systemConfig.networkConfig.ntp == true);
            CHECK(systemConfig.networkConfig.ntpServer == sacnlogger::AddressOrHostname("us.pool.ntp.org"));
        }
        CHECK(systemConfig.networkConfig.dhcp == true);
        CHECK(systemConfig.networkConfig.address == etcpal::IpAddr::FromString("192.168.1.204"));
        CHECK(systemConfig.networkConfig.mask == etcpal::IpAddr::FromString("255.255.0.0"));
        CHECK(systemConfig.networkConfig.gateway == etcpal::IpAddr::FromString("192.168.1.1"));
    }

    SECTION("Static")
    {
        SECTION("Without NTP")
        {
            eth0Object
                ->addVTable(
                    sdbus::registerMethod(describeMethod)
                        .implementedAs(ReturnFileContents(RESOURCES_PATH "/SystemConfigTest/network/static.json")))
                .forInterface(linkInterface);

            systemConfig.readFromSystem();
            CHECK(systemConfig.networkConfig.ntp == false);
            CHECK(systemConfig.networkConfig.ntpServer == sacnlogger::AddressOrHostname{});
        }
        SECTION("With NTP")
        {
            eth0Object
                ->addVTable(
                    sdbus::registerMethod(describeMethod)
                        .implementedAs(ReturnFileContents(RESOURCES_PATH "/SystemConfigTest/network/static-ntp.json")))
                .forInterface(linkInterface);

            systemConfig.readFromSystem();
            CHECK(systemConfig.networkConfig.ntp == true);
            CHECK(systemConfig.networkConfig.ntpServer == sacnlogger::AddressOrHostname("us.pool.ntp.org"));
        }
        CHECK(systemConfig.networkConfig.dhcp == false);
        CHECK(systemConfig.networkConfig.address == etcpal::IpAddr::FromString("192.168.1.204"));
        CHECK(systemConfig.networkConfig.mask == etcpal::IpAddr::FromString("255.255.0.0"));
        CHECK(systemConfig.networkConfig.gateway == etcpal::IpAddr::FromString("192.168.1.1"));
    }
}
