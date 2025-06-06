/**
 * @file Config.h
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

#ifndef CONFIG_H
#define CONFIG_H

#ifdef SACNLOGGER_EMBEDDED_BUILD
#define SACNLOGGER_SYSTEM_CONFIG
#endif

#include <cstdint>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>
#ifdef SACNLOGGER_SYSTEM_CONFIG
#include "SystemConfig.h"
#endif

namespace sacnlogger
{

    /**
     * System configuration.
     */
    class Config
    {
    public:
        bool operator==(const Config&) const = default;

        std::vector<uint16_t> universes;
        bool usePap = false;
#ifdef SACNLOGGER_SYSTEM_CONFIG
        SystemConfig systemConfig;
#endif

        static Config loadFromFile(const std::string& filename);
        void saveToFile(const std::string& filename) const;
    };

    void to_json(nlohmann::json& j, const Config& value);
    void from_json(const nlohmann::json& j, Config& value);

} // namespace sacnlogger

#endif // CONFIG_H
