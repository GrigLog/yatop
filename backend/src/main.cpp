#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#include "HttpServer.h"
#include "ResourceSampler.h"


struct AppConfig {
    ServerConfig server;
    std::chrono::milliseconds interval = std::chrono::milliseconds(1000);
};

void printUsage() {
    std::cout
        << "Usage: yatop [--host 127.0.0.1] [--port 8080] "
        << "[--frontend-root frontend/dist] [--interval-ms 1000]\n";
}

bool parseInt(std::string value, int& target) {
    char* end = nullptr;
    auto parsed = std::strtol(value.c_str(), &end, 10);
    if (end == value.c_str() || *end != '\0')
        return false;

    target = static_cast<int>(parsed);
    return true;
}

std::optional<AppConfig> parseArgs(int argc, char** argv) {
    auto config = AppConfig();

    for (auto index = 1; index < argc; ++index) {
        auto arg = std::string(argv[index]);
        auto needsValue = index + 1 < argc;

        if (arg == "--help") {
            printUsage();
            return std::nullopt;
        } else if (arg == "--host" && needsValue) {
            config.server.host = argv[++index];
        } else if (arg == "--port" && needsValue) {
            if (!parseInt(argv[++index], config.server.port))
                return std::nullopt;
        } else if (arg == "--frontend-root" && needsValue) {
            config.server.webRoot = argv[++index];
        } else if (arg == "--interval-ms" && needsValue) {
            auto intervalMs = 0;
            if (!parseInt(argv[++index], intervalMs) || intervalMs < 100)
                return std::nullopt;
            config.interval = std::chrono::milliseconds(intervalMs);
        } else {
            std::cerr << "Unknown or incomplete argument: " << arg << "\n";
            printUsage();
            return std::nullopt;
        }
    }

    return config;
}

int main(int argc, char** argv) {
    auto configOpt = parseArgs(argc, argv);
    if (!configOpt)
        return 1;

    if (!std::filesystem::exists(configOpt->server.webRoot)) {
        std::cerr << "frontend root does not exist: " << configOpt->server.webRoot << "\n";
        return 1;
    }

    ResourceSampler sampler(configOpt->interval);
    sampler.start();

    HttpServer server(configOpt->server, sampler);
    return server.run() ? 0 : 1;
}
