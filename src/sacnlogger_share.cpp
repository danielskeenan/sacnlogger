/**
 * @file sacnlogger_share.cpp
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

#include "sacnlogger_share.h"
#include <boost/dll/runtime_symbol_info.hpp>
#include <cstdlib>
#include "sacnlogger_config.h"

namespace sacnlogger::config
{
    std::filesystem::path shareDir() noexcept
    {
        static std::filesystem::path resolvedShareDir;
        if (resolvedShareDir.empty())
        {
            const auto envShareDir = std::getenv("SACNLOGGER_SHAREDIR");
            if (envShareDir == nullptr || envShareDir == "")
            {
                // Use default.
                resolvedShareDir = (boost::dll::program_location().parent_path() / kDefaultShareDir).string();
            }
            else
            {
                // Use from environment.
                resolvedShareDir = envShareDir;
            }
        }
        return resolvedShareDir;
    }

    std::filesystem::path templateDir() { return shareDir() / "system_config" / "templates/"; }

    std::filesystem::path systemPrefix()
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

} // namespace sacnlogger::config
