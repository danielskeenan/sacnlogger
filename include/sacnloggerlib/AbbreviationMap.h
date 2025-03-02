/**
 * @file AbbreviationMap.h
 *
 * @author Dan Keenan
 * @date 3/2/25
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

#ifndef ABBREVIATIONMAP_H
#define ABBREVIATIONMAP_H

#include <etcpal/cpp/uuid.h>
#include <string>
#include <unordered_map>

namespace sacnlogger
{

    /**
     * Store an increasing abbreviation for a given UUID.
     *
     * Starts with `A`, followed by `B`, `C`, ..., `Z`, `AA`, `AB`, ...
     */
    class AbbreviationMap
    {
    public:
        std::string abbreviationForUuid(const etcpal::Uuid& cid);

    private:
        std::unordered_map<etcpal::Uuid, std::string> abbreviations_;
        std::string nextAbbreviation_ = "A";
    };

} // namespace sacnlogger

#endif // ABBREVIATIONMAP_H
