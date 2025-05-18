/**
 * @file sacnlogger_share.h
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

#ifndef SACNLOGGER_SHARE_H
#define SACNLOGGER_SHARE_H
#include <filesystem>

namespace sacnlogger::config
{
    /**
     * Get path to shared data files.
     *
     * Defaults to `/usr/share/sacnlogger`.
     *
     * To override at runtime, set the env var `SACNLOGGER_SHAREDIR`.
     */
    std::filesystem::path shareDir() noexcept;

    /**
     * Path to file templates.
     */
    std::filesystem::path templateDir();

    /**
     * Path to file schemas.
     */
    std::filesystem::path schemaDir();

    /**
     * Path to prefix where files should be installed.
     *
     * To change this for runtime, set the environment variable `SACNLOGGER_SYS_PREFIX`.
     *
     * Defaults to `/`.
     */
    std::filesystem::path systemPrefix();
} // namespace sacnlogger::config

#endif // SACNLOGGER_SHARE_H
