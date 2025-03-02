/**
 * @file AbbreviationMapTest.cpp
 *
 * @author Dan Keenan
 * @date 3/2/25
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
#include <fmt/format.h>
#include "etcpal/cpp/uuid.h"
#include "sacnloggerlib/AbbreviationMap.h"

static const std::vector<std::string> kLetters{
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
    "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
};
static const std::vector<std::string> kExpectedAbbreviations = []()
{
    // Build sequence that looks like <char0><char1><char2>.
    auto char0List = kLetters;
    char0List.insert(char0List.begin(), 2, "");
    auto char1List = kLetters;
    char1List.insert(char1List.begin(), "");
    auto char2List = kLetters;

    std::vector<std::string> r;
    for (const auto& char0 : char0List)
    {
        for (const auto& char1 : char1List)
        {
            for (const auto& char2 : char2List)
            {
                r.emplace_back(fmt::format("{}{}{}", char0, char1, char2));
            }
        }
    }

    return r;
}();

TEST_CASE("Abbreviation Map")
{
    sacnlogger::AbbreviationMap abbreviationMap;
    const auto nsUuid = etcpal::Uuid::OsPreferred();

    SECTION("Sequence")
    {
        for (const auto& expectedAbbreviation : kExpectedAbbreviations)
        {
            const auto uuid = etcpal::Uuid::V5(nsUuid, expectedAbbreviation);
            CHECK(abbreviationMap.abbreviationForUuid(uuid) == expectedAbbreviation);
        }
    }

    SECTION("Repetition")
    {
        const auto uuid0 = etcpal::Uuid::V5(nsUuid, kExpectedAbbreviations.at(0));
        const auto uuid1 = etcpal::Uuid::V5(nsUuid, kExpectedAbbreviations.at(1));

        CHECK(abbreviationMap.abbreviationForUuid(uuid0) == kExpectedAbbreviations.at(0));
        CHECK(abbreviationMap.abbreviationForUuid(uuid1) == kExpectedAbbreviations.at(1));
        CHECK(abbreviationMap.abbreviationForUuid(uuid0) == kExpectedAbbreviations.at(0));
    }
}
