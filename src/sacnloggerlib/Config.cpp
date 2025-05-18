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
#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>
#include <sacn/common.h>
#include <sacn/cpp/common.h>
#include <sacnlogger_share.h>
#include <spdlog/spdlog.h>
#include "sacnloggerlib/ConfigException.h"

namespace sacnlogger
{
    constexpr auto kUniverses = "universes";
    constexpr auto kUsePap = "usePap";
    constexpr auto kSystem = "system";

    const nlohmann::json_schema::json_validator& getValidator()
    {
        static std::optional<nlohmann::json_schema::json_validator> validator;
        if (!validator)
        {
            std::ifstream f(config::schemaDir() / "sacnlogger.schema.json");
            static nlohmann::json schema = nlohmann::json::parse(f);
            validator.emplace(nullptr, nlohmann::json_schema::default_string_format_check);
            validator->set_root_schema(schema);
        }
        return *validator;
    }

    Config Config::loadFromFile(const std::string& filename)
    {
        nlohmann::json json;
        try
        {
            std::ifstream f(filename, std::ios::binary);
            json = nlohmann::json::parse(f);
        }
        catch (const std::exception& e)
        {
            SPDLOG_CRITICAL("Error parsing config file: {}", e.what());
            throw ConfigException("Error parsing config file", e);
        }

        // Validate.
        const auto& validator = getValidator();
        try
        {
            // Apply default values.
            const auto patch = validator.validate(json);
            json.patch_inplace(patch);
        }
        catch (const std::exception& e)
        {
            SPDLOG_CRITICAL("Invalid config file: {}", e.what());
            throw ConfigException("Invalid config file", e);
        }

        // Load.
        Config config;
        json[kUniverses].get_to(config.universes);
        json[kUsePap].get_to(config.usePap);
#ifdef SACNLOGGER_SYSTEM_CONFIG
        if (json.contains(kSystem))
        {
            json[kSystem].get_to(config.systemConfig);
        }
#endif

        return config;
    }

    void Config::saveToFile(const std::string& filename) const
    {
        nlohmann::json json;
        json[kUniverses] = universes;
        json[kUsePap] = usePap;
#ifdef SACNLOGGER_SYSTEM_CONFIG
        json[kSystem] = systemConfig;
#endif

        // Validate resulting output only when building Debug build.
#ifndef NDEBUG
        const auto& validator = getValidator();
        validator.validate(json);
#endif

        try
        {
            std::ofstream f(filename, std::ios::binary);
            f << json;
        }
        catch (const std::exception& e)
        {
            SPDLOG_CRITICAL("Error saving config file: {}", e.what());
            throw ConfigException("Error saving config file", e);
        }
    }
} // namespace sacnlogger
