/**
 * @file UniverseMonitor.h
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

#ifndef UNIVERSEMONITOR_H
#define UNIVERSEMONITOR_H

#include <cstdint>
#include <memory>
#include <sacn/cpp/merge_receiver.h>
#include <set>
#include <spdlog/logger.h>
#include <unordered_set>
#include "AbbreviationMap.h"

namespace sacnlogger
{
    /**
     * Deleter for sacn::MergeReceiver objects.
     */
    struct MergeReceiverDeleter
    {
        void operator()(sacn::MergeReceiver* mergeReceiver) const
        {
            mergeReceiver->Shutdown();
            delete mergeReceiver;
        }
    };

    /**
     * Collection of incoming sACN data.
     *
     * Contains levels, priorities, and owners for comparison to determine if data has changed from the previous packet.
     */
    struct ComparableData
    {
        auto operator<=>(const ComparableData&) const = default;
        ComparableData() = default;
        explicit ComparableData(const SacnRecvMergedData& mergedData);

        std::array<uint8_t, SACN_MERGE_RECEIVER_MAX_SLOTS> levels_{};
        std::array<uint8_t, SACN_MERGE_RECEIVER_MAX_SLOTS> priorities_{};
        std::array<sacn_remote_source_t, SACN_MERGE_RECEIVER_MAX_SLOTS> owners_{};
    };

    /**
     * Collection of incoming sACN sources.
     */
    struct ComparableSources
    {
        struct ComparableSource
        {
            std::weak_ordering operator<=>(const ComparableSource& rhs) const;
            friend bool operator==(const ComparableSource& lhs, const ComparableSource& rhs)
            {
                return std::tie(lhs.cid, lhs.name) == std::tie(rhs.cid, rhs.name);
            }
            friend bool operator!=(const ComparableSource& lhs, const ComparableSource& rhs) { return !(lhs == rhs); }
            etcpal::Uuid cid;
            std::string name;
        };
        std::weak_ordering operator<=>(const ComparableSources&) const = default;
        ComparableSources() = default;
        explicit ComparableSources(sacn::MergeReceiver* mergeReceiver, const SacnRecvMergedData& mergedData);
        std::set<ComparableSource> sources_{};
    };

    /**
     * Handle incoming universe data.
     */
    class UniverseNotifyHandler : public sacn::MergeReceiver::NotifyHandler
    {
    public:
        explicit UniverseNotifyHandler(sacn::MergeReceiver* mergeReceiver, spdlog::logger* sourceLogger,
                                       spdlog::logger* dataLogger) :
            mergeReceiver_(mergeReceiver), sourceLogger_(sourceLogger), dataLogger_(dataLogger)
        {
        }

        void HandleMergedData(sacn::MergeReceiver::Handle handle, const SacnRecvMergedData& mergedData) override;
        void HandleNonDmxData(sacn::MergeReceiver::Handle receiverHandle, const etcpal::SockAddr& sourceAddr,
                              const SacnRemoteSource& sourceInfo, const SacnRecvUniverseData& universeData) override;
        void HandleSourcesLost(sacn::MergeReceiver::Handle handle, uint16_t universe,
                               const std::vector<SacnLostSource>& lostSources) override;

    private:
        ComparableData lastData_;
        ComparableSources lastSources_;
        sacn::MergeReceiver* mergeReceiver_;
        spdlog::logger* sourceLogger_;
        spdlog::logger* dataLogger_;
        AbbreviationMap abbreviationMap_;
    };

    /**
     * Monitor a single universe of sACN for changes.
     */
    class UniverseMonitor
    {
    public:
        explicit UniverseMonitor(uint16_t universe) : universe_(universe) {}
        [[nodiscard]] uint16_t universe() const { return universe_; }
        void setUniverse(uint16_t universe) { universe_ = universe; }
        [[nodiscard]] bool usePap() const { return usePap_; }
        void setUsePap(bool usePap) { usePap_ = usePap; }

        void start();

    private:
        static constexpr auto kLoggerPattern = "%Y-%m-%d %H:%M:%S.%e%z,%v";

        std::shared_ptr<spdlog::logger> sourceLogger_;
        std::shared_ptr<spdlog::logger> dataLogger_;
        std::unique_ptr<sacn::MergeReceiver, MergeReceiverDeleter> mergeReceiver_;
        std::unique_ptr<UniverseNotifyHandler> notifyHandler_;
        unsigned int maxLogFileCount = 99;
        unsigned long maxLogFileSize = 20971520; // 20 MB
        uint16_t universe_;
        bool usePap_ = false;
    };

} // namespace sacnlogger

#endif // UNIVERSEMONITOR_H
