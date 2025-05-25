/**
 * @file WebServer.h
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

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <crow/app.h>
#include <crow/middlewares/cors.h>

#include "CrowLogHandler.h"

namespace sacnlogger
{
    class WebServer
    {
    public:
        explicit WebServer();

        void run();
        void stop();
        std::string getUrl();

    private:
        using CrowServer = crow::Crow<crow::CORSHandler>;

        CrowServer server_;
        CrowLogHandler crowLogHandler_;
        std::future<void> serverHandle_;

        /**
         * Set the response @p res to serve a static file @p path relative to sacnlogger::config::webDir().
         * @param res
         * @param path
         */
        static void crowStaticFile(crow::response& res, const std::string& path);
    };
} // namespace sacnlogger

#endif // WEBSERVER_H
