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
#include <nlohmann/json_fwd.hpp>
#include "CrowLogHandler.h"

namespace sacnlogger
{
    class Runner;

    namespace detail
    {
        struct ErrorMessage
        {
            explicit ErrorMessage() = default;

            virtual ~ErrorMessage() = default;
            virtual unsigned int code() = 0;
            virtual const char* name() const = 0;
            virtual std::string message() const = 0;
            void addToResponse(crow::response& res);

            nlohmann::json json() const;
        };

        struct UnsupportedTypeError : ErrorMessage
        {
            explicit UnsupportedTypeError(const std::vector<std::string>& allowedMimeTypes) :
                ErrorMessage(), allowedMimeTypes_(allowedMimeTypes)
            {
            }

            unsigned int code() override { return crow::status::UNSUPPORTED_MEDIA_TYPE; }
            const char* name() const override { return "UnsupportedTypeError"; }
            std::string message() const override;

        private:
            const std::vector<std::string> allowedMimeTypes_;
        };

        struct UnprocessableContentError : ErrorMessage
        {
            explicit UnprocessableContentError(const std::string& why) : ErrorMessage(), why_(why) {}

            unsigned int code() override { return 422; }
            const char* name() const override { return "UnprocessableContentError"; }
            std::string message() const override;

        private:
            std::string why_;
        };
    } // namespace detail

    class WebServer
    {
    public:
        explicit WebServer(Runner* runner);

        void run();
        void stop();
        std::string getUrl();

    private:
        using CrowServer = crow::Crow<crow::CORSHandler>;

        Runner* runner_;
        CrowServer server_;
        CrowLogHandler crowLogHandler_;
        std::future<void> serverHandle_;

        /**
         * Set the response @p res to serve a static file @p path relative to sacnlogger::config::webDir().
         * @param res
         * @param path
         */
        static void rtStaticFile(crow::response& res, const std::string& path);
        static void rtIndex(crow::response& res) { rtStaticFile(res, "index.html"); }
        crow::response rtConfig(const crow::request& req);

        static bool sendErrorIfNotOctetStream(const crow::request& req, crow::response& res);
        static void sendSuccess(const crow::request& req, crow::response& res);
    };
} // namespace sacnlogger

#endif // WEBSERVER_H
