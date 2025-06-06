/**
 * @file main.cpp
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

#include <catch2/catch_session.hpp>

int main(int argc, char* argv[])
{
    // Setup
#ifdef PLATFORM_LINUX
    setenv("SACNLOGGER_SHAREDIR", SACNLOGGER_SHAREDIR, true);
    setenv("SACNLOGGER_SYS_PREFIX", SACNLOGGER_SYS_PREFIX, true);
#endif

    int result = Catch::Session().run(argc, argv);

    // Teardown

    return result;
}
