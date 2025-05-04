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
#include "FileMatcher.h"

TEST_CASE("Network Write")
{
    sacnlogger::SystemConfig systemConfig;

    SECTION("DHCP")
    {
        systemConfig.networkConfig.dhcp = true;

        SECTION("Without NTP")
        {
            systemConfig.networkConfig.ntp = false;
            systemConfig.writeToSystem();
            CHECK_THAT(RESOURCES_PATH "/SystemConfigTest/network/dhcp.network",
                       EqualsFile(SACNLOGGER_SYS_PREFIX "/etc/systemd/network/01-eth0.network"));
        }
        SECTION("With NTP")
        {
            systemConfig.networkConfig.ntp = true;
            systemConfig.writeToSystem();
            CHECK_THAT(RESOURCES_PATH "/SystemConfigTest/network/dhcp-ntp.network",
                       EqualsFile(SACNLOGGER_SYS_PREFIX "/etc/systemd/network/01-eth0.network"));
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
            systemConfig.writeToSystem();
            CHECK_THAT(RESOURCES_PATH "/SystemConfigTest/network/static.network",
                       EqualsFile(SACNLOGGER_SYS_PREFIX "/etc/systemd/network/01-eth0.network"));
        }
        SECTION("With NTP")
        {
            systemConfig.networkConfig.ntp = true;
            systemConfig.networkConfig.ntpServer = "us.pool.ntp.org";
            systemConfig.writeToSystem();
            CHECK_THAT(RESOURCES_PATH "/SystemConfigTest/network/static-ntp.network",
                       EqualsFile(SACNLOGGER_SYS_PREFIX "/etc/systemd/network/01-eth0.network"));
        }
    }
}
