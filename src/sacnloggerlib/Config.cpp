/**
 * @file Config.cpp
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

#include "sacnloggerlib/Config.h"
#include <libconfig.h++>
#include <sacn/cpp/common.h>
#include <spdlog/spdlog.h>

#include <sacn/common.h>
#include "sacnloggerlib/ConfigException.h"

namespace sacnlogger
{
    constexpr auto kApplication = "application";
    constexpr auto kUniverses = "universes";
    constexpr auto kUsePap = "usePap";

    Config Config::loadFromFile(const std::string& filename)
    {
        Config config;

        // Load file.
        libconfig::Config c;
        try
        {
            c.readFile(filename);
        }
        catch (const libconfig::FileIOException& e)
        {
            SPDLOG_CRITICAL("Error reading config file - {}", e.what());
            throw ConfigException("Error reading config file.", e);
        }
        catch (const libconfig::ParseException& e)
        {
            SPDLOG_CRITICAL("Error parsing config file {}:{} - {}", e.getFile(), e.getLine(), e.getError());
            throw ConfigException("Error parsing config file.", e);
        }

        // Read config.
        try
        {
            // Application.
            const auto& appConfig = c.lookup(kApplication);
            // Universes.
            const auto& universes = appConfig.lookup(kUniverses);
            for (const auto& cfgUniverse : universes)
            {
                const unsigned int universe = cfgUniverse;
                if (universe < sacn::kMinimumUniverse || universe > sacn::kMaximumUniverse)
                {
                    SPDLOG_CRITICAL("Universe out of range - {}", universe);
                    throw ConfigException("Universe out of range.");
                }
                if (std::ranges::find(config.universes, universe) != config.universes.end())
                {
                    // Duplicate universe.
                    SPDLOG_CRITICAL("Malformed config file: Universe '{}' is listed more than once.", universe);
                    throw ConfigException("Malformed config file: Universe is listed more than once.");
                }
                config.universes.push_back(universe);
            }
            // Use PaP
            if (appConfig.exists(kUsePap))
            {
                config.usePap = appConfig.lookup(kUsePap);
            }
        }
        catch (const libconfig::ConfigException& e)
        {
            SPDLOG_CRITICAL("Error reading application config - {}", e.what());
            throw ConfigException("Error reading application config.", e);
        }

        return config;
    }

    void Config::saveToFile(const std::string& filename) const
    {
        libconfig::Config c;
        auto& root = c.getRoot();

        // Application.
        auto& appConfig = root.add(kApplication, libconfig::Setting::TypeGroup);
        auto& universesConfig = appConfig.add(kUniverses, libconfig::Setting::TypeArray);
        for (const auto universe : universes)
        {
            universesConfig.add(libconfig::Setting::TypeInt) = universe;
        }
        appConfig.add(kUsePap, libconfig::Setting::TypeBoolean) = usePap;

        try
        {
            c.writeFile(filename);
        }
        catch (const libconfig::FileIOException& e)
        {
            SPDLOG_CRITICAL("Error writing config file - {}", filename);
            throw ConfigException("Error writing config file", e);
        }
    }
} // namespace sacnlogger
