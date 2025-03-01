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
#include <fstream>
#include <sacn/cpp/common.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include "sacnloggerlib/ConfigException.h"

namespace sacnlogger
{
    static constexpr auto kConfUniverses = "universes";
    static constexpr auto kConfUsePap = "usePap";
    Config Config::loadFromFile(const std::string& filename)
    {
        Config config;

        // Load file.
        YAML::Node yaml;
        try
        {
            yaml = YAML::LoadFile(filename);
        }
        catch (const YAML::Exception& e)
        {
            SPDLOG_CRITICAL("Error loading config file: {}.", e.what());
            throw ConfigException("Error loading config file.", e);
        }

        // Verify format.
        if (!yaml.IsMap())
        {
            SPDLOG_CRITICAL("Malformed config file: Not a map.");
            throw ConfigException("Malformed config file: Not a map.");
        }

        // Universes.
        if (!yaml[kConfUniverses].IsSequence())
        {
            SPDLOG_CRITICAL("Malformed config file: 'universes` not present or wrong format.");
            throw ConfigException("Malformed config file: 'universes` not present or wrong format.");
        }
        for (const auto& universe : yaml[kConfUniverses])
        {
            if (!universe.IsScalar())
            {
                SPDLOG_CRITICAL("Malformed config file: Universe '{}' is invalid.", universe.as<std::string>());
                throw ConfigException("Malformed config file: Universe is invalid.");
            }
            const auto universeInt = universe.as<int>();
            if (universeInt > sacn::kMaximumUniverse || universeInt < sacn::kMinimumUniverse)
            {
                SPDLOG_CRITICAL("Malformed config file: Universe '{}' is out of range.", universeInt);
                throw ConfigException("Malformed config file: Universe is out of range.");
            }
            if (std::ranges::find(config.universes, universeInt) != config.universes.end())
            {
                // Duplicate universe.
                SPDLOG_CRITICAL("Malformed config file: Universe '{}' is listed more than once.", universeInt);
                throw ConfigException("Malformed config file: Universe is listed more than once.");
            }
            config.universes.push_back(universeInt);
        }

        // Use PAP
        if (yaml[kConfUsePap].IsScalar())
        {
            config.usePap = yaml[kConfUsePap].as<bool>();
        }

        return config;
    }

    void Config::saveToFile(const std::string& filename) const
    {
        YAML::Node yaml;
        yaml[kConfUniverses] = universes;
        yaml[kConfUsePap] = usePap;

        std::ofstream out(filename);
        if (!out.is_open())
        {
            SPDLOG_CRITICAL("Error writing config file: {}", filename);
            throw std::runtime_error("Error writing config file");
        }
        out << yaml;
    }
} // namespace sacnlogger
