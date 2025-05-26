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
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>
#include <sacnloggerlib/ConfigException.h>
#include <spdlog/spdlog.h>
#include "sacnlogger_share.h"
#include "sacnloggerlib/Runner.h"

namespace sacnlogger
{

    namespace detail
    {
        nlohmann::json ErrorMessage::json() const
        {
            return {
                {"name", name()},
                {"message", message()},
            };
        }

        void ErrorMessage::addToResponse(crow::response& res)
        {
            res.code = code();
            res.set_header("Content-Type", "application/json");
            res.body = json().dump();
        }

        std::string UnsupportedTypeError::message() const
        {
            return fmt::format("Supported types: {}", allowedMimeTypes_);
        }

        std::string UnprocessableContentError::message() const { return fmt::format("Bad content format: {}", why_); }
    } // namespace detail

    WebServer::WebServer(Runner* runner) : runner_(runner)
    {
        // Setup HTTP server.
        crow::logger::setHandler(&crowLogHandler_);
        // Crow will try to listen to signals by default, but we manage them ourselves.
        server_.port(80).multithreaded().signal_clear();
        auto& cors = server_.get_middleware<crow::CORSHandler>();
        cors.global().methods(crow::HTTPMethod::Get).origin("*");

        // Config
        CROW_ROUTE(server_, "/rpc/config")
            .methods(crow::HTTPMethod::Get,
                     crow::HTTPMethod::POST)(std::bind(&WebServer::rtConfig, this, std::placeholders::_1));

        // Static content.
        CROW_ROUTE(server_, "/").methods(crow::HTTPMethod::Get)(&WebServer::rtIndex);
        CROW_ROUTE(server_, "/<path>").methods(crow::HTTPMethod::Get)(&WebServer::rtStaticFile);
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

    void WebServer::rtStaticFile(crow::response& res, const std::string& path)
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

    crow::response WebServer::rtConfig(const crow::request& req)
    {
        crow::response res;
        if (sendErrorIfNotOctetStream(req, res))
        {
            return res;
        }

        if (req.method == crow::HTTPMethod::GET)
        {
            res.set_header("Content-Type", "application/octet-stream");
            flatbuffers::FlatBufferBuilder fbb;
            const auto msg = runner_->config().saveToMessage();
            fbb.Finish(message::Config::Pack(fbb, msg.get()));
            res.body.assign(fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize());
            return res;
        }
        else if (req.method == crow::HTTPMethod::POST)
        {
            const std::span data(reinterpret_cast<const uint8_t*>(req.body.data()), req.body.size());
            flatbuffers::Verifier verifier(data.data(), data.size());
            if (!message::VerifyConfigBuffer(verifier))
            {
                detail::UnprocessableContentError("Invalid format.").addToResponse(res);
                return res;
            }
            try
            {
                const std::unique_ptr<message::ConfigT> msg(message::GetConfig(data.data())->UnPack());
                const auto newConfig = Config::loadFromMessage(msg);
                configSaver_ = std::async(std::launch::async,
                                          [this, newConfig]()
                                          {
                                              // Wait to apply config until response is sent.
                                              SPDLOG_INFO("Reconfiguring, will restart in 10 seconds....");
                                              std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                                              runner_->setConfig(newConfig);
                                              SPDLOG_INFO("Completed reconfiguring.");
                                          });
            }
            catch (const std::exception& e)
            {
                detail::UnprocessableContentError(fmt::format("Bad message: {}", e.what())).addToResponse(res);
                return res;
            }
        }
        sendSuccess(req, res);
        return res;
    }

    bool WebServer::sendErrorIfNotOctetStream(const crow::request& req, crow::response& res)
    {
        if (req.method == crow::HTTPMethod::GET)
        {
            if (req.get_header_value("Accept") != "application/octet-stream")
            {
                detail::UnsupportedTypeError({"application/octet-stream"}).addToResponse(res);
                res.end();
                return true;
            }
        }
        else if (req.method == crow::HTTPMethod::POST)
        {
            if (req.get_header_value("Content-Type") != "application/octet-stream")
            {
                detail::UnsupportedTypeError({"application/octet-stream"}).addToResponse(res);
                res.set_header("Accept-Post", "application/octet-stream");
                res.end();
                return true;
            }
        }

        return false;
    }

    void WebServer::sendSuccess(const crow::request& req, crow::response& res)
    {
        res.set_header("Content-Type", "text/plain");
        res.code = crow::status::OK;
        res.body = "success";
    }

} // namespace sacnlogger
