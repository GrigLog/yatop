#pragma once

#include <filesystem>
#include <optional>
#include <vector>

#include "Metrics.h"


class ProcReader {
public:
    std::filesystem::path procRoot;
    long pageSize = 4096;

public:
    explicit ProcReader(std::filesystem::path procRoot = "/proc");

    CpuTimes readCpuTimes();
    MemoryMetrics readMemory();
    std::vector<ProcessRawSample> readProcesses();

protected:
    std::optional<ProcessRawSample> readProcess(int pid);
};
