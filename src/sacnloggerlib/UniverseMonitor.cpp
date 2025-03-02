/**
 * @file UniverseMonitor.cpp
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

#include "sacnloggerlib/UniverseMonitor.h"
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <sacnloggerlib/CsvRow.h>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

#include <memory>

namespace sacnlogger
{
    ComparableData::ComparableData(const SacnRecvMergedData& mergedData)
    {
        const auto addrStartOffset = mergedData.slot_range.start_address - 1;
        const auto addrCount = mergedData.slot_range.address_count;
        if (addrStartOffset + addrCount > levels_.size())
        {
            SPDLOG_CRITICAL("Received more slots than expected: Got {}, wanted no more than {}", addrCount,
                            levels_.size());
            return;
        }
        std::memcpy(levels_.data() + addrStartOffset, mergedData.levels, addrCount);
        std::memcpy(priorities_.data() + addrStartOffset, mergedData.priorities, addrCount);
        std::memcpy(owners_.data() + (addrStartOffset * sizeof(sacn_remote_source_t)), mergedData.owners,
                    addrCount * sizeof(sacn_remote_source_t));
    }

    ComparableSources::ComparableSources(sacn::MergeReceiver* mergeReceiver, const SacnRecvMergedData& mergedData)
    {
        for (std::size_t ix = 0; ix < mergedData.num_active_sources; ++ix)
        {
            const auto sourceHandle = mergedData.active_sources[ix];
            if (const auto source = mergeReceiver->GetSource(sourceHandle))
            {
                sources_.emplace(source->cid, source->addr, source->name);
            }
        }
    }

    void UniverseNotifyHandler::HandleMergedData(sacn::MergeReceiver::Handle handle,
                                                 const SacnRecvMergedData& mergedData)
    {
        ComparableSources newSources(mergeReceiver_, mergedData);
        if (newSources != lastSources_)
        {
            // Sources have changed!
            for (const auto& newSource : newSources.sources_)
            {
                if (!lastSources_.sources_.contains(newSource))
                {
                    std::string action = "started";
                    const auto ipAddr = newSource.ipAddr.ip().ToString();
                    const auto ipAddrIt = cidIpAddrMap_.find(newSource.cid);
                    if (ipAddrIt != cidIpAddrMap_.end())
                    {
                        action = "moved";
                    }
                    cidIpAddrMap_.insert_or_assign(newSource.cid, ipAddr);
                    CsvRow row;
                    row << action << abbreviationMap_.abbreviationForUuid(newSource.cid) << newSource.cid.ToString()
                        << ipAddr << newSource.name;
                    sourceLogger_->info(row.string());
                }
            }
            lastSources_ = std::move(newSources);
        }

        ComparableData newData(mergedData);
        if (newData != lastData_)
        {
            // Data has changed!
            std::unordered_map<sacn_remote_source_t, std::string> sourceNames;
            for (std::size_t ix = 0; ix < mergedData.num_active_sources; ++ix)
            {
                const auto sourceHandle = mergedData.active_sources[ix];
                if (const auto source = mergeReceiver_->GetSource(sourceHandle))
                {
                    sourceNames.emplace(sourceHandle, abbreviationMap_.abbreviationForUuid(source->cid));
                }
            }

            CsvRow row;
            auto levelsIt = newData.levels_.cbegin();
            auto prioritiesIt = newData.priorities_.cbegin();
            auto ownersIt = newData.owners_.cbegin();
            for (; levelsIt != newData.levels_.cend(); ++levelsIt, ++prioritiesIt, ++ownersIt)
            {
                const auto sourceName = *ownersIt == sacn::kInvalidRemoteSourceHandle ? "-" : sourceNames.at(*ownersIt);
                row << static_cast<unsigned int>(*levelsIt) << static_cast<unsigned int>(*prioritiesIt) << sourceName;
            }
            dataLogger_->info(row.string());
            lastData_ = std::move(newData);
        }
    }

    void UniverseNotifyHandler::HandleNonDmxData(sacn::MergeReceiver::Handle receiverHandle,
                                                 const etcpal::SockAddr& sourceAddr, const SacnRemoteSource& sourceInfo,
                                                 const SacnRecvUniverseData& universeData)
    {
        // Do nothing.
    }

    void UniverseNotifyHandler::HandleSourcesLost(sacn::MergeReceiver::Handle handle, uint16_t universe,
                                                  const std::vector<SacnLostSource>& lostSources)
    {
        for (const auto& source : lostSources)
        {
            const etcpal::Uuid sourceCid(etcpal::Uuid(source.cid));
            const auto sourceIpAddr = [this, &sourceCid]() -> std::string
            {
                const auto sourceIpAddrIt = cidIpAddrMap_.find(sourceCid);
                if (sourceIpAddrIt == cidIpAddrMap_.end())
                {
                    return "Unknown";
                }
                return sourceIpAddrIt->second;
            }();
            CsvRow row;
            row << "stopped" << abbreviationMap_.abbreviationForUuid(source.cid) << etcpal::Uuid(source.cid).ToString()
                << sourceIpAddr << source.name;
            sourceLogger_->info(row.string());
            cidIpAddrMap_.erase(sourceCid);
        }
    }

    void UniverseMonitor::start()
    {
        // Setup loggers.
        // Source logger.
        const auto sourceLoggerName = fmt::format("U{:05d}_sources", universe_);
        sourceLogger_ = spdlog::rotating_logger_mt<spdlog::async_factory>(
            sourceLoggerName, fmt::format("{}.csv", sourceLoggerName), maxLogFileSize, maxLogFileCount);
        sourceLogger_->set_pattern(kLoggerPattern);

        // Data logger.
        const auto dataLoggerName = fmt::format("U{:05d}_data", universe_);
        dataLogger_ = spdlog::rotating_logger_mt<spdlog::async_factory>(
            dataLoggerName, fmt::format("{}.csv", dataLoggerName), maxLogFileSize, maxLogFileCount);
        dataLogger_->set_pattern(kLoggerPattern);

        // Log a header line as a marker for beginning of monitoring.
        sourceLogger_->info("State,Marker,CID,IP Address,Name");
        CsvRow dataLoggerHeader;
        for (unsigned int addr = 1; addr <= SACN_MERGE_RECEIVER_MAX_SLOTS; ++addr)
        {
            dataLoggerHeader << fmt::format("{:03d} Lvl", addr) << fmt::format("{:03d} Pri", addr)
                             << fmt::format("{:03d} Src", addr);
        }
        dataLogger_->info(dataLoggerHeader.string());

        // Setup merge receiver.
        sacn::MergeReceiver::Settings settings(universe_);
        settings.use_pap = usePap_;
        mergeReceiver_.reset(new sacn::MergeReceiver);
        notifyHandler_ =
            std::make_unique<UniverseNotifyHandler>(mergeReceiver_.get(), sourceLogger_.get(), dataLogger_.get());
        const auto err = mergeReceiver_->Startup(settings, *notifyHandler_);
        if (!err.IsOk())
        {
            SPDLOG_CRITICAL("Failed to start merge receiver for universe {}: {}", universe_, err.ToString());
        }
    }

} // namespace sacnlogger
