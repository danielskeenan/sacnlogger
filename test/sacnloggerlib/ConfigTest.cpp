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
#include <map>
#include <sacnloggerlib/ConfigException.h>

#include "sacnloggerlib/Config.h"

static const std::map<std::string, sacnlogger::Config> expectedConfigs{
    {"one_univ.yaml", {.universes = {1}, .usePap = false}},
    {"five_univ.yaml", {.universes = {1, 2, 3, 4, 5}, .usePap = false}},
    {"use_pap.yaml", {.universes = {1}, .usePap = true}},
};

namespace Catch
{
    template <>
    struct StringMaker<sacnlogger::Config>
    {
        static std::string convert(const sacnlogger::Config& config)
        {
            return fmt::format("<Config: Univs {}, PAP {}>", config.universes, config.usePap);
        }
    };
} // namespace Catch

TEST_CASE("Load config file", "[Config]")
{
    SECTION("Good configs")
    {
        const auto [filename, expected] = GENERATE(Catch::Generators::from_range(expectedConfigs));
        const auto filePath = fmt::format("{}/ConfigTest/{}", RESOURCES_PATH, filename);
        sacnlogger::Config actual;
        REQUIRE_NOTHROW(actual = sacnlogger::Config::loadFromFile(filePath));
        REQUIRE(expected == actual);
    }

    SECTION("Bad configs")
    {
        SECTION("bad_univ.yaml")
        {
            const auto filePath = fmt::format("{}/ConfigTest/{}", RESOURCES_PATH, "bad_univ.yaml");
            sacnlogger::Config actual;
            REQUIRE_THROWS_AS(sacnlogger::Config::loadFromFile(filePath), sacnlogger::ConfigException);
        }
        SECTION("malformed.yaml")
        {
            const auto filePath = fmt::format("{}/ConfigTest/{}", RESOURCES_PATH, "malformed.yaml");
            sacnlogger::Config actual;
            REQUIRE_THROWS_AS(sacnlogger::Config::loadFromFile(filePath), sacnlogger::ConfigException);
        }
        SECTION("doubled_univ.yaml")
        {
            const auto filePath = fmt::format("{}/ConfigTest/{}", RESOURCES_PATH, "doubled_univ.yaml");
            sacnlogger::Config actual;
            REQUIRE_THROWS_AS(sacnlogger::Config::loadFromFile(filePath), sacnlogger::ConfigException);
        }
    }
}
