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

#include <cstdint>
#include <string>
#include <vector>

namespace sacnlogger
{

    /**
     * System configuration.
     */
    class Config
    {
    public:
        auto operator<=>(const Config&) const = default;

        std::vector<uint16_t> universes;
        bool usePap = false;

        static Config loadFromFile(const std::string& filename);
        void saveToFile(const std::string& filename) const;
    };

} // namespace sacnlogger

#endif // CONFIG_H
