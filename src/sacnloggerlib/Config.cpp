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


constexpr auto kUniverses = "universes";
constexpr auto kUsePap = "usePap";
constexpr auto kSystem = "system";

namespace sacnlogger
{
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

    void to_json(nlohmann::json& j, const Config& value)
    {
        j = nlohmann::json{
            {kUniverses, value.universes},
            {kUsePap, value.usePap},
#ifdef SACNLOGGER_SYSTEM_CONFIG
            {kSystem, value.systemConfig},
#endif
        };
    }

    void from_json(const nlohmann::json& j, Config& value)
    {
        nlohmann::json::const_iterator it;
        if ((it = j.find(kUniverses)) != j.end())
        {
            it->get_to(value.universes);
        }
        if ((it = j.find(kUsePap)) != j.end())
        {
            it->get_to(value.usePap);
        }
#ifdef SACNLOGGER_SYSTEM_CONFIG
        if ((it = j.find(kSystem)) != j.end())
        {
            it->get_to(value.systemConfig);
        }
#endif
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

        return loadFromJson(json);
    }

    Config Config::loadFromString(const std::string& str)
    {
        try
        {
            const nlohmann::json json(str);
            return loadFromJson(json);
        }
        catch (const std::exception& e)
        {
            SPDLOG_CRITICAL("Error parsing config string: {}", e.what());
            throw ConfigException("Error parsing config string", e);
        }
    }

    Config Config::loadFromJson(const nlohmann::json& json)
    {
        // Validate.
        nlohmann::json mergedJson;
        const auto& validator = getValidator();
        try
        {
            // Apply default values.
            const auto patch = validator.validate(json);
            mergedJson = json.patch(patch);
        }
        catch (const std::exception& e)
        {
            SPDLOG_CRITICAL("Invalid config file: {}", e.what());
            throw ConfigException("Invalid config file", e);
        }

        // Load.
        Config config;
        mergedJson.get_to(config);

        return config;
    }

    void Config::saveToFile(const std::string& filename) const
    {
        nlohmann::json json = *this;

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

    std::unique_ptr<message::ConfigT> Config::saveToMessage() const
    {
        auto msg = std::make_unique<message::ConfigT>();
        msg->universes = universes;
        msg->usePap = usePap;
#ifdef SACNLOGGER_SYSTEM_CONFIG
        msg->system = systemConfig.saveToMessage();
#endif
        return msg;
    }

    Config Config::loadFromMessage(const std::unique_ptr<message::ConfigT>& msg)
    {
        Config config;
        if (std::ranges::any_of(msg->universes, [](const decltype(message::ConfigT::universes)::value_type val)
                                { return val < sacn::kMinimumUniverse || val > sacn::kMaximumUniverse; }))
        {
            throw ConfigException("Universe out of range");
        }
        config.universes = msg->universes;
        config.usePap = msg->usePap;
#ifdef SACNLOGGER_SYSTEM_CONFIG
        config.systemConfig.readFromMessage(msg->system);
#endif

        return config;
    }
} // namespace sacnlogger
