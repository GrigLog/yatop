#include "ResourceSampler.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <mutex>
#include <sys/sysinfo.h>
#include <utility>


ResourceSampler::ResourceSampler(std::chrono::milliseconds interval) :
    interval(interval) {
    cpuCount = get_nprocs();
    if (cpuCount < 1)
        cpuCount = 1;
}

ResourceSampler::~ResourceSampler() {
    if (worker.joinable())
        worker.request_stop();
}

void ResourceSampler::start() {
    collect();
    worker = std::jthread([this](auto stopToken) {
        run(stopToken);
    });
}

SystemSnapshot ResourceSampler::snapshot() {
    auto lock = std::shared_lock(snapshotMutex);
    return currentSnapshot;
}

void ResourceSampler::run(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        std::this_thread::sleep_for(interval); //todo: use sleep_until
        collect();
    }
}

SystemSnapshot ResourceSampler::collect() {
    auto cpuTimes = procReader.readCpuTimes();
    auto memory = procReader.readMemory();
    auto rawProcesses = procReader.readProcesses();
    auto snapshot = SystemSnapshot();

    auto now = std::chrono::system_clock::now();
    //todo: more precise clock
    snapshot.timestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    snapshot.memory = memory;

    auto totalDelta = cpuTimes.total() - previousCpuTimes.total();
    auto idleDelta = cpuTimes.idleAll() - previousCpuTimes.idleAll();
    if (hasPreviousSample && totalDelta > 0) {
        auto activeDelta = totalDelta > idleDelta ? totalDelta - idleDelta : 0;
        snapshot.cpu.totalPercent = static_cast<double>(activeDelta) * 100.0 / static_cast<double>(totalDelta);
    }

    snapshot.processes.reserve(rawProcesses.size());
    auto nextProcessTicks = std::unordered_map<int, std::uint64_t>();
    for (auto rawProcess : rawProcesses) {
        ProcessMetrics process;
        process.raw = rawProcess;

        auto previous = previousProcessTicks.find(rawProcess.pid);
        if (hasPreviousSample && previous != previousProcessTicks.end() && totalDelta > 0) {
            auto processDelta = rawProcess.cpuTicks >= previous->second ? rawProcess.cpuTicks - previous->second : 0;
            process.cpuPercent = static_cast<double>(processDelta) * 100.0 * cpuCount / static_cast<double>(totalDelta);
        }

        nextProcessTicks[rawProcess.pid] = rawProcess.cpuTicks;
        snapshot.processes.push_back(std::move(process));
    }

    std::ranges::sort(snapshot.processes, [](auto left, auto right) {
        if (std::fabs(left.cpuPercent - right.cpuPercent) > 0.001)
            return left.cpuPercent > right.cpuPercent;
        return left.raw.pid < right.raw.pid;
    });

    {
        auto lock = std::unique_lock(snapshotMutex);
        currentSnapshot = snapshot;
        previousCpuTimes = cpuTimes;
        previousProcessTicks = std::move(nextProcessTicks);
        hasPreviousSample = true;
    }

    return snapshot;
}
