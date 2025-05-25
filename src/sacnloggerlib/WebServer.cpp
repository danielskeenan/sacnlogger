/**
 * @file WebServer.cpp
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

#include "sacnloggerlib/WebServer.h"
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "sacnlogger_share.h"

namespace sacnlogger
{
    WebServer::WebServer()
    {
        // Setup HTTP server.
        crow::logger::setHandler(&crowLogHandler_);
        server_.port(80).multithreaded();
        auto& cors = server_.get_middleware<crow::CORSHandler>();
        cors.global().methods(crow::HTTPMethod::Get).origin("*");

        // Static content.
        CROW_ROUTE(server_, "/")
            .methods(crow::HTTPMethod::Get)([](crow::response& res) { crowStaticFile(res, "index.html"); });
        CROW_ROUTE(server_, "/<path>")
            .methods(crow::HTTPMethod::Get)([this](crow::response& res, const std::string& path)
                                            { crowStaticFile(res, path); });
    }

    void WebServer::run()
    {
        // Start with the port defined in the constructor. If that fails, increase nextPort until it works.
        unsigned int nextPort = 5050;
        for (;;)
        {
            serverHandle_ = server_.run_async();
            const auto status = server_.wait_for_server_start();
            if (status != std::cv_status::timeout)
            {
                break;
            }
            server_.port(++nextPort);
        }
        SPDLOG_INFO("Web server listening on {}", getUrl());
    }

    void WebServer::stop()
    {
        server_.stop();
        serverHandle_.wait();
        SPDLOG_INFO("Web server stopped");
    }

    std::string WebServer::getUrl() { return fmt::format("http://{}:{}", server_.bindaddr(), server_.port()); }

    void WebServer::crowStaticFile(crow::response& res, const std::string& path)
    {
        auto cleanPath = path;
        crow::utility::sanitize_filename(cleanPath);
        // Directory index.
        const auto absPath = config::webDir() / cleanPath;
        if (std::filesystem::is_directory(absPath))
        {
            res.redirect(fmt::format("/{}/index.html", path));
        }
        else
        {
            res.set_static_file_info_unsafe(absPath);
            if (res.code == 404)
            {
                // Serve index.html so client-side routing can work.
                res.set_static_file_info_unsafe(config::webDir() / "index.html");
            }
        }

        res.end();
    }

} // namespace sacnlogger
