#include "HttpServer.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <utility>

#include "cpp-httplib/httplib.h"


HttpServer::HttpServer(ServerConfig config, ResourceSampler& sampler) :
    config(std::move(config)),
    sampler(sampler) {
}

bool HttpServer::run() {
    auto server = httplib::Server();

    server.Get("/api/snapshot", [this](auto, auto& response) {
        response.set_content(sampler.snapshot().toJson().dump(), "application/json");
    });

    server.Get("/api/events", [this](auto, auto& response) {
        response.set_header("Cache-Control", "no-cache");
        response.set_header("Connection", "keep-alive");
        response.set_chunked_content_provider("text/event-stream", [this](auto, auto& sink) {
            while (sink.is_writable()) {
                auto payload = std::string("data: ") + sampler.snapshot().toJson().dump() + "\n\n";
                if (!sink.write(payload.data(), payload.size()))
                    break;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            return true;
        });
    });

    if (!server.set_mount_point("/", config.webRoot.string())) {
        std::cerr << "Unable to mount frontend root: " << config.webRoot << "\n";
        return false;
    }

    std::cout << "yatop is listening on http://" << config.host << ":" << config.port << "\n";
    return server.listen(config.host, config.port);
}
