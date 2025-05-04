/**
 * @file SystemConfig.h
 *
 * @author Dan Keenan
 * @date 5/3/25
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

#ifndef SYSTEMCONFIG_H
#define SYSTEMCONFIG_H

#include <etcpal/cpp/inet.h>
#include <filesystem>
#include <nlohmann/json_fwd.hpp>
#include "AddressOrHostname.h"

namespace sacnlogger
{
    namespace detail
    {
        struct NetworkConfig
        {
            bool dhcp = false;
            etcpal::IpAddr address;
            etcpal::IpAddr mask;
            etcpal::IpAddr gateway;
            bool ntp = false;
            AddressOrHostname ntpServer;
        };

        void to_json(nlohmann::json& j, const NetworkConfig& value);
        void from_json(const nlohmann::json& j, NetworkConfig& value);
    } // namespace detail
    /**
     * Read/Write system hardware configuration.
     */
    class SystemConfig
    {
    public:
        detail::NetworkConfig networkConfig;

        void readFromSystem();
        void writeToSystem();

    private:
        /**
         * Path to file templates.
         */
        static std::filesystem::path templateDir();
        /**
         * Path to prefix where files should be installed.
         *
         * To change this for runtime, set the environment variable `SACNLOGGER_SYS_PREFIX`.
         *
         * Defaults to `/`.
         */
        static std::filesystem::path systemPrefix();
        /** Path to systemd-network interface configuration file relative to systemPrefix(). */
        static std::filesystem::path netConfigFile();
    };

    void to_json(nlohmann::json& j, const SystemConfig& value);
    void from_json(const nlohmann::json& j, SystemConfig& value);
} // namespace sacnlogger

#endif // SYSTEMCONFIG_H
