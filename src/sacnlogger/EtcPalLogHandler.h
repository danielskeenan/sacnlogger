/**
 * @file SpdLogEtcPalLogHandler.h
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

#ifndef SPDLOGETCPALLOGHANDLER_H
#define SPDLOGETCPALLOGHANDLER_H

#include <etcpal/cpp/log.h>

namespace sacnlogger
{
    /**
     * Sends EtcPal log messages to spdlog.
     */
    class EtcPalLogHandler : public etcpal::LogMessageHandler
    {
    public:
        etcpal::LogTimestamp GetLogTimestamp() override;
        void HandleLogMessage(const EtcPalLogStrings& strings) override;
    };

    /**
     * Deleter for etcpal::Logger objects.
     */
    struct EtcPalLoggerDeleter
    {
        void operator()(etcpal::Logger* logger) const
        {
            logger->Shutdown();
            delete logger;
        }
    };
} // namespace sacnlogger

#endif // SPDLOGETCPALLOGHANDLER_H
