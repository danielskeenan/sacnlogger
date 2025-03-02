/**
 * @file CsvRowTest.cpp
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
#include <sacnloggerlib/CsvRow.h>

TEST_CASE("CSV Row")
{
    SECTION("3 ints")
    {
        sacnlogger::CsvRow row;
        row << 1 << 2 << 3;
        REQUIRE(row.string() == "1,2,3");
    }

    SECTION("3 strings")
    {
        sacnlogger::CsvRow row;
        row << "one" << "two" << "three";
        REQUIRE(row.string() == "\"one\",\"two\",\"three\"");
    }

    SECTION("Embedded quotes")
    {
        sacnlogger::CsvRow row;
        row << "here" << "is" << "a \" quote";
        REQUIRE(row.string() == "\"here\",\"is\",\"a \"\" quote\"");
    }

    SECTION("Multiple datatypes")
    {
        sacnlogger::CsvRow row;
        row << 3 << "blind" << "mice";
        REQUIRE(row.string() == "3,\"blind\",\"mice\"");
    }

    SECTION("Empty row")
    {
        sacnlogger::CsvRow row;
        REQUIRE(row.string() == "");
    }
}
