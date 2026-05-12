#include "ProcReader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <unistd.h>


std::uint64_t CpuTimes::total() {
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

std::uint64_t CpuTimes::idleAll() {
    return idle + iowait;
}

ProcReader::ProcReader(std::filesystem::path procRoot) :
    procRoot(std::move(procRoot)) {
    pageSize = sysconf(_SC_PAGESIZE);
}

CpuTimes ProcReader::readCpuTimes() {
    auto file = std::ifstream(procRoot / "stat");
    auto label = std::string();
    auto times = CpuTimes();

    file >> label;
    if (label != "cpu")
        return times;

    file >> times.user
        >> times.nice
        >> times.system
        >> times.idle
        >> times.iowait
        >> times.irq
        >> times.softirq
        >> times.steal;

    return times;
}

MemoryMetrics ProcReader::readMemory() {
    auto file = std::ifstream(procRoot / "meminfo");
    auto key = std::string();
    auto unit = std::string();
    auto valueKb = std::uint64_t();
    auto totalKb = std::uint64_t();
    auto availableKb = std::uint64_t();

    while (file >> key >> valueKb >> unit) {
        if (key == "MemTotal:")
            totalKb = valueKb;
        else if (key == "MemAvailable:")
            availableKb = valueKb;
    }

    auto memory = MemoryMetrics();
    memory.totalBytes = totalKb * 1024;
    memory.availableBytes = availableKb * 1024;
    memory.usedBytes = memory.totalBytes > memory.availableBytes ? memory.totalBytes - memory.availableBytes : 0;

    if (memory.totalBytes > 0)
        memory.usedPercent = static_cast<double>(memory.usedBytes) * 100.0 / static_cast<double>(memory.totalBytes);

    return memory;
}

std::vector<ProcessRawSample> ProcReader::readProcesses() {
    auto processes = std::vector<ProcessRawSample>();

    for (auto entry : std::filesystem::directory_iterator(procRoot)) {
        if (!entry.is_directory())
            continue;

        auto name = entry.path().filename().string();
        if (!std::ranges::all_of(name, [](auto ch) { return std::isdigit(ch); }))
            continue;

        auto process = readProcess(std::stoi(name));
        if (process)
            processes.push_back(*process);
    }

    return processes;
}

std::optional<ProcessRawSample> ProcReader::readProcess(int pid) {
    auto statFile = std::ifstream(procRoot / std::to_string(pid) / "stat");
    auto line = std::string();
    if (!std::getline(statFile, line))
        return std::nullopt;

    auto openName = line.find('(');
    auto closeName = line.rfind(')');
    if (openName == std::string::npos || closeName == std::string::npos || closeName <= openName)
        return std::nullopt;

    auto process = ProcessRawSample();
    process.pid = pid;
    process.name = line.substr(openName + 1, closeName - openName - 1);

    auto rest = std::istringstream(line.substr(closeName + 2));
    auto fields = std::vector<std::string>();
    auto field = std::string();
    while (rest >> field)
        fields.push_back(field);

    if (fields.size() < 22)
        return std::nullopt;

    process.state = fields[0].empty() ? '?' : fields[0][0];
    auto userTicks = std::stoull(fields[11]);
    auto systemTicks = std::stoull(fields[12]);
    auto rssPages = std::stoll(fields[21]);

    process.cpuTicks = userTicks + systemTicks;
    process.memoryBytes = rssPages > 0 ? static_cast<std::uint64_t>(rssPages) * static_cast<std::uint64_t>(pageSize) : 0;

    return process;
}
