/**
 * @file AddressOrHostname.h
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

#ifndef ADDRESSORHOSTNAME_H
#define ADDRESSORHOSTNAME_H

#include <etcpal/cpp/inet.h>
#include <variant>

namespace sacnlogger
{
    /**
     * Store either an IP Address (192.0.2.1) or a Hostname (example.com).
     */
    class AddressOrHostname
    {
    public:
        bool operator==(const AddressOrHostname&) const = default;
        AddressOrHostname() = default;
        explicit AddressOrHostname(const std::string& value);
        explicit AddressOrHostname(const etcpal::IpAddr& value);
        AddressOrHostname(const AddressOrHostname&) = default;
        AddressOrHostname(AddressOrHostname&&) = default;
        AddressOrHostname& operator=(const AddressOrHostname&) = default;
        template <typename T>
        AddressOrHostname& operator=(const T& other) noexcept
        {
            setValue(other);
            return *this;
        }
        AddressOrHostname& operator=(AddressOrHostname&& other) = default;
        template <typename T>
        AddressOrHostname& operator=(T&& other) noexcept
        {
            setValue(std::forward<T>(other));
            return *this;
        }

        [[nodiscard]] bool isValid() const;
        [[nodiscard]] std::string toString() const;

    private:
        void setValue(const std::string& value);
        void setValue(const etcpal::IpAddr& value);

        using ValueType = std::variant<std::monostate, std::string, etcpal::IpAddr>;
        struct IsValidVisitor
        {
            bool operator()(std::monostate) const { return false; }
            bool operator()(const std::string& value) const;
            bool operator()(const etcpal::IpAddr& value) const;
        };
        struct ToStringVisitor
        {
            std::string operator()(std::monostate) const { return {}; }
            std::string operator()(const std::string& value) const;
            std::string operator()(const etcpal::IpAddr& value) const;
        };

        ValueType value_;
    };
} // namespace sacnlogger

#endif // ADDRESSORHOSTNAME_H
