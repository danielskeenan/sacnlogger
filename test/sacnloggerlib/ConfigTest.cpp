/**
 * @file ConfigTest.cpp
 *
 * @author Dan Keenan
 * @date 3/1/25
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
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <sacnloggerlib/ConfigException.h>
#include "sacnloggerlib/Config.h"

static const std::map<std::string, sacnlogger::Config> expectedConfigs{
    {"one_univ.json", {.universes = {1}, .usePap = false}},
    {"five_univ.json", {.universes = {1, 2, 3, 4, 5}, .usePap = false}},
    {"use_pap.json", {.universes = {1}, .usePap = true}},
};

namespace Catch
{
    template <>
    struct StringMaker<sacnlogger::Config>
    {
        static std::string convert(const sacnlogger::Config& config)
        {
#ifdef SACNLOGGER_SYSTEM_CONFIG
            const auto systemConfig = fmt::format(
                ", Network dhcp {}, {}/{}/{}, ntp {}, {}", config.systemConfig.networkConfig.dhcp,
                config.systemConfig.networkConfig.address.ToString(), config.systemConfig.networkConfig.mask.ToString(),
                config.systemConfig.networkConfig.gateway.ToString(), config.systemConfig.networkConfig.ntp,
                config.systemConfig.networkConfig.ntpServer.toString());
#else
            const auto systemConfig = "";
#endif
            return fmt::format("<Config: Univs {}, PAP {}{}>", config.universes, config.usePap, systemConfig);
        }
    };
} // namespace Catch

TEST_CASE("Config Load")
{
    SECTION("Good configs")
    {
        const auto [filename, expected] = GENERATE(Catch::Generators::from_range(expectedConfigs));
        const auto filePath = fmt::format("{}/ConfigTest/{}", RESOURCES_PATH, filename);
        sacnlogger::Config actual;
        REQUIRE_NOTHROW(actual = sacnlogger::Config::loadFromFile(filePath));
        REQUIRE(expected == actual);
    }

#ifdef SACNLOGGER_SYSTEM_CONFIG
    SECTION("Network Static")
    {
        const auto filePath = fmt::format("{}/ConfigTest/{}", RESOURCES_PATH, "network_static.json");

        sacnlogger::Config expected{
            .universes = {1},
            .usePap = false,
        };
        expected.systemConfig.networkConfig.dhcp = false;
        expected.systemConfig.networkConfig.address = etcpal::IpAddr::FromString("192.168.1.101");
        expected.systemConfig.networkConfig.mask = etcpal::IpAddr::FromString("255.255.255.0");
        expected.systemConfig.networkConfig.gateway = etcpal::IpAddr::FromString("192.168.1.1");
        expected.systemConfig.networkConfig.ntp = true;
        expected.systemConfig.networkConfig.ntpServer = "us.pool.ntp.org";

        sacnlogger::Config actual;
        REQUIRE_NOTHROW(actual = sacnlogger::Config::loadFromFile(filePath));
        REQUIRE(expected == actual);
    }

    SECTION("Network DHCP")
    {
        const auto filePath = fmt::format("{}/ConfigTest/{}", RESOURCES_PATH, "network_dhcp.json");

        sacnlogger::Config expected{
            .universes = {1},
            .usePap = false,
        };
        expected.systemConfig.networkConfig.dhcp = true;

        sacnlogger::Config actual;
        REQUIRE_NOTHROW(actual = sacnlogger::Config::loadFromFile(filePath));
        REQUIRE(expected == actual);
    }
#endif

    SECTION("Bad configs")
    {
        SECTION("bad_univ.json")
        {
            const auto filePath = fmt::format("{}/ConfigTest/{}", RESOURCES_PATH, "bad_univ.json");
            sacnlogger::Config actual;
            REQUIRE_THROWS_AS(sacnlogger::Config::loadFromFile(filePath), sacnlogger::ConfigException);
        }
        SECTION("malformed.json")
        {
            const auto filePath = fmt::format("{}/ConfigTest/{}", RESOURCES_PATH, "malformed.json");
            sacnlogger::Config actual;
            REQUIRE_THROWS_AS(sacnlogger::Config::loadFromFile(filePath), sacnlogger::ConfigException);
        }
        SECTION("doubled_univ.json")
        {
            const auto filePath = fmt::format("{}/ConfigTest/{}", RESOURCES_PATH, "doubled_univ.json");
            sacnlogger::Config actual;
            REQUIRE_THROWS_AS(sacnlogger::Config::loadFromFile(filePath), sacnlogger::ConfigException);
        }
    }
}

TEST_CASE("Config Save")
{
    // Start with default values.
    nlohmann::json expected = sacnlogger::Config();
    nlohmann::json actual;
    SECTION("Standard")
    {
        const auto [expectedFilename, expectedConfig] = GENERATE(Catch::Generators::from_range(expectedConfigs));
        const auto expectedFilePath = fmt::format("{}/ConfigTest/{}", RESOURCES_PATH, expectedFilename);
        std::ifstream expectedFile(expectedFilePath);
        REQUIRE(expectedFile.is_open());
        expected.merge_patch(nlohmann::json::parse(expectedFile));
        actual = expectedConfig;
    }

#ifdef SACNLOGGER_SYSTEM_CONFIG
    SECTION("Network Static")
    {
        const auto expectedFilePath = fmt::format("{}/ConfigTest/{}", RESOURCES_PATH, "network_static.json");
        std::ifstream expectedFile(expectedFilePath);
        REQUIRE(expectedFile.is_open());
        expected.merge_patch(nlohmann::json::parse(expectedFile));

        sacnlogger::Config expectedConfig{
            .universes = {1},
            .usePap = false,
        };
        expectedConfig.systemConfig.networkConfig.dhcp = false;
        expectedConfig.systemConfig.networkConfig.address = etcpal::IpAddr::FromString("192.168.1.101");
        expectedConfig.systemConfig.networkConfig.mask = etcpal::IpAddr::FromString("255.255.255.0");
        expectedConfig.systemConfig.networkConfig.gateway = etcpal::IpAddr::FromString("192.168.1.1");
        expectedConfig.systemConfig.networkConfig.ntp = true;
        expectedConfig.systemConfig.networkConfig.ntpServer = "us.pool.ntp.org";

        actual = expectedConfig;
    }

    SECTION("Network DHCP")
    {
        const auto expectedFilePath = fmt::format("{}/ConfigTest/{}", RESOURCES_PATH, "network_dhcp.json");
        std::ifstream expectedFile(expectedFilePath);
        REQUIRE(expectedFile.is_open());
        expected.merge_patch(nlohmann::json::parse(expectedFile));

        sacnlogger::Config expectedConfig{
            .universes = {1},
            .usePap = false,
        };
        expectedConfig.systemConfig.networkConfig.dhcp = true;

        actual = expectedConfig;
    }
#endif

    CHECK(expected == actual);
}
