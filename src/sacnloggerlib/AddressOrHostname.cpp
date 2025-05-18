/**
 * @file AddressOrHostname.cpp
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

#include "../../include/sacnloggerlib/SystemConfig/AddressOrHostname.h"
#include <regex>

namespace sacnlogger
{
    // https://stackoverflow.com/a/20204811 Captures FQDN and hostnames.
    static const std::regex hostnameRe(
        R"((?=^.{4,253}$)((^((?!-)[a-zA-Z0-9-]{0,62}[a-zA-Z0-9])+)|(^((?!-)[a-zA-Z0-9-]{0,62}[a-zA-Z0-9]\.)+[a-zA-Z]{2,63}))$)",
        std::regex::optimize);

    AddressOrHostname::AddressOrHostname(const std::string& value) { setValue(value); }

    AddressOrHostname::AddressOrHostname(const etcpal::IpAddr& value) { setValue(value); }

    bool AddressOrHostname::isValid() const { return std::visit(IsValidVisitor(), value_); }

    std::string AddressOrHostname::toString() const { return std::visit(ToStringVisitor(), value_); }

    void AddressOrHostname::setValue(const std::string& value)
    {
        // Try IP Address first.
        auto ipAddress = etcpal::IpAddr::FromString(value);
        if (ipAddress.IsValid())
        {
            value_ = ipAddress;
            return;
        }

        // Hostname.
        if (std::regex_match(value, hostnameRe))
        {
            value_ = value;
            return;
        }
    }

    void AddressOrHostname::setValue(const etcpal::IpAddr& value) { value_ = value; }

    bool AddressOrHostname::IsValidVisitor::operator()(const std::string& value) const
    {
        return std::regex_match(value, hostnameRe);
    }

    bool AddressOrHostname::IsValidVisitor::operator()(const etcpal::IpAddr& value) const { return value.IsValid(); }

    std::string AddressOrHostname::ToStringVisitor::operator()(const std::string& value) const { return value; }

    std::string AddressOrHostname::ToStringVisitor::operator()(const etcpal::IpAddr& value) const
    {
        return value.ToString();
    }


} // namespace sacnlogger
