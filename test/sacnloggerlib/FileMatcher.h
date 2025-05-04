/**
 * @file FileMatcher.h
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

#ifndef FILEMATCHER_H
#define FILEMATCHER_H

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

class FileMatcher : public Catch::Matchers::MatcherGenericBase
{
private:
    static constexpr auto kBufSize = 1024;

public:
    FileMatcher(const std::string& actualPath) : actualPath_(actualPath) {}

    bool match(const std::string& expectedPath) const
    {
        // Compare sizes.
        if (std::filesystem::file_size(expectedPath) != std::filesystem::file_size(actualPath_))
        {
            return false;
        }

        // Compare contents.
        std::ifstream expectedFile(expectedPath, std::ios::binary);
        if (!expectedFile.is_open())
        {
            throw std::runtime_error("Could not open " + expectedPath);
        }
        std::ifstream actualFile(actualPath_, std::ios::binary);
        if (!actualFile.is_open())
        {
            throw std::runtime_error("Could not open " + actualPath_);
        }
        std::vector<char> expectedBuf(kBufSize);
        std::vector<char> actualBuf(kBufSize);
        while (expectedFile.read(expectedBuf.data(), expectedBuf.size()) &&
               actualFile.read(actualBuf.data(), actualBuf.size()))
        {
            const auto expectedCount = expectedFile.gcount();
            const auto actualCount = actualFile.gcount();
            if (!std::equal(expectedBuf.begin(), expectedBuf.begin() + expectedCount, actualBuf.begin(),
                            actualBuf.begin() + actualCount))
            {
                return false;
            }
        }

        return true;
    }

    std::string describe() const override { return "Contents equal: " + actualPath_; }

private:
    std::string actualPath_;
};

inline FileMatcher EqualsFile(const std::string& actualPath) { return FileMatcher(actualPath); }

#endif // FILEMATCHER_H
