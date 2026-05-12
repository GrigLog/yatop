#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"


struct CpuTimes {
    std::uint64_t user = 0;
    std::uint64_t nice = 0;
    std::uint64_t system = 0;
    std::uint64_t idle = 0;
    std::uint64_t iowait = 0;
    std::uint64_t irq = 0;
    std::uint64_t softirq = 0;
    std::uint64_t steal = 0;

    std::uint64_t total();
    std::uint64_t idleAll();
};

struct CpuMetrics {
    double totalPercent = 0.0;
};

struct MemoryMetrics {
    std::uint64_t totalBytes = 0;
    std::uint64_t usedBytes = 0;
    std::uint64_t availableBytes = 0;
    double usedPercent = 0.0;
};

struct ProcessRawSample {
    int pid = 0;
    std::string name;
    char state = '?';
    std::uint64_t cpuTicks = 0;
    std::uint64_t memoryBytes = 0;
};

//I wanted to keep ProcessRawSample immutable, so I added this new class for the cpuPercent field
struct ProcessMetrics {
    ProcessRawSample raw;
    double cpuPercent = 0.0;
};

struct SystemSnapshot {
    std::int64_t timestampMs = 0;
    CpuMetrics cpu;
    MemoryMetrics memory;
    std::vector<ProcessMetrics> processes;

    nlohmann::json toJson() const {
        auto json = nlohmann::json();
        json["timestampMs"] = timestampMs;
        json["cpu"] = {
            { "totalPercent", cpu.totalPercent },
        };
        json["memory"] = {
            { "totalBytes", memory.totalBytes },
            { "usedBytes", memory.usedBytes },
            { "availableBytes", memory.availableBytes },
            { "usedPercent", memory.usedPercent },
        };
        json["processes"] = nlohmann::json::array();

        for (auto process : processes) {
            json["processes"].push_back({
                { "pid", process.raw.pid },
                { "name", process.raw.name },
                { "state", std::string(1, process.raw.state) },
                { "cpuPercent", process.cpuPercent },
                { "memoryBytes", process.raw.memoryBytes },
            });
        }

        return json;
    }
};
