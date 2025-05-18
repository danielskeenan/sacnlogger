/**
 * @file FakeDbus.h
 *
 * @author Dan Keenan
 * @date 5/18/25
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

#ifndef FAKEDBUS_H
#define FAKEDBUS_H

#include <sdbus-c++/sdbus-c++.h>

/**
 * Fake a DBus server. See
 * https://github.com/Kistler-Group/sdbus-cpp/blob/master/docs/using-sdbus-c%2B%2B.md#using-direct-peer-to-peer-d-bus-connections
 */
class FakeDbus
{
private:
    std::array<int, 2> fds_{};

public:
    std::shared_ptr<sdbus::IConnection> client;
    std::unique_ptr<sdbus::IConnection> server;

    FakeDbus()
    {
        socketpair(AF_UNIX, SOCK_STREAM, 0, fds_.data());
        std::thread t(
            [&]()
            {
                server = sdbus::createServerBus(fds_[0]);
                // This is necessary so that createDirectBusConnection() below does not block
                server->enterEventLoopAsync();
            });
        client = sdbus::createDirectBusConnection(fds_[1]);
        client->enterEventLoopAsync();
        t.join();
    }

    ~FakeDbus()
    {
        if (client)
        {
            client->leaveEventLoop();
        }
        if (server)
        {
            server->leaveEventLoop();
        }
    }
};

#endif // FAKEDBUS_H
