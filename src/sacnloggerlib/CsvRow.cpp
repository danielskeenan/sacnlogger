/**
 * @file CsvRow.cpp
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

#include "sacnloggerlib/CsvRow.h"

namespace sacnlogger
{
    CsvRow& CsvRow::operator<<(const std::string& val)
    {
        stream_ << '"';
        for (const auto c : val)
        {
            stream_ << c;
            // Escape quotation mark with additional quotation mark.
            if (c == '"')
            {
                stream_ << '"';
            }
        }
        stream_ << "\",";

        return *this;
    }

    std::string CsvRow::string() const
    {
        auto s = stream_.str();
        s.pop_back();
        return s;
    }
} // namespace sacnlogger
