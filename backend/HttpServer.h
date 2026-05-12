#pragma once

#include <filesystem>
#include <string>

#include "ResourceSampler.h"


struct ServerConfig {
    std::string host = "127.0.0.1";
    int port = 8080;
    std::filesystem::path webRoot = "frontend/dist";
};

struct HttpServer {
    const ServerConfig config;
    ResourceSampler& sampler;

public:
    HttpServer(ServerConfig config, ResourceSampler& sampler);
    bool run();
};
