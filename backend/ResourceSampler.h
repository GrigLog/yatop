#pragma once

#include <chrono>
#include <shared_mutex>
#include <thread>
#include <unordered_map>

#include "nlohmann/json.hpp"

#include "Metrics.h"
#include "ProcReader.h"


struct ResourceSampler {
    ProcReader procReader;
    std::jthread worker;
    std::shared_mutex snapshotMutex;
    CpuTimes previousCpuTimes;
    std::unordered_map<int, std::uint64_t> previousProcessTicks;
    bool hasPreviousSample = false;

    std::chrono::milliseconds interval;
    int cpuCount = 1;

protected:
    SystemSnapshot currentSnapshot;

public:
    explicit ResourceSampler(std::chrono::milliseconds interval);
    ~ResourceSampler();

    void start();
    SystemSnapshot snapshot();

protected:
    void run(std::stop_token stopToken);
    SystemSnapshot collect();
};
