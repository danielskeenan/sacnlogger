/**
 * @file CrowLogHandler.h
 *
 * @author Dan Keenan
 * @date 5/25/25
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

#ifndef CROWLOGHANDLER_H
#define CROWLOGHANDLER_H

#include <crow/logging.h>

namespace sacnlogger
{
    /**
     * Send Crow log messages to spdlog.
     */
    class CrowLogHandler : public crow::ILogHandler
    {
    public:
        void log(std::string message, crow::LogLevel crowLogLevel) override;
    };
} // namespace sacnlogger

#endif // CROWLOGHANDLER_H
