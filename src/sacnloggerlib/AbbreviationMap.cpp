/**
 * @file AbbreviationMap.cpp
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

#include "sacnloggerlib/AbbreviationMap.h"

namespace sacnlogger
{
    std::string AbbreviationMap::abbreviationForUuid(const etcpal::Uuid& cid)
    {
        auto& abbreviation = abbreviations_[cid];
        if (abbreviation.empty())
        {
            abbreviation = nextAbbreviation_;
            bool incremented = false;
            for (auto it = nextAbbreviation_.rbegin(); !incremented && it != nextAbbreviation_.rend(); ++it)
            {
                if (*it == 'Z')
                {
                    *it = 'A';
                }
                else
                {
                    ++(*it);
                    incremented = true;
                }
            }
            if (!incremented)
            {
                nextAbbreviation_.insert(nextAbbreviation_.begin(), 'A');
            }
        }
        return abbreviation;
    }
} // namespace sacnlogger
