// HTTP front door for the autocorrect backend, built on cpp-httplib.
// Uses Trie and RedBlackTree from DSA-Project-2 exactly as they are.
//
//   GET  /api/suggest?prefix=ap&limit=5&engine=trie|rbt  ->  {"engine":"trie","suggestions":["apple","app",...],"micros":12}
//   POST /api/accept?word=apple          ->  {"ok":true}
//   GET  /api/health                     ->  {"ok":true}
//
// Optionally serves the built React app (../autocorrect-app/dist) at "/".

#include "server_core.h"

#include "httplib.h"  // https://github.com/yhirose/cpp-httplib (single header)

#include <chrono>
#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    const std::string wordsPath = argc > 1 ? argv[1] : "words.txt";
    const int port = argc > 2 ? std::atoi(argv[2]) : 8080;
    const std::string staticDir = argc > 3 ? argv[3] : "";

    AutocorrectService service;
    try {
        std::cout << "Loaded " << service.loadWords(wordsPath) << " words\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    httplib::Server svr;

    svr.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("{\"ok\":true}", "application/json");
    });

    svr.Get("/api/suggest", [&](const httplib::Request& req, httplib::Response& res) {
        // Clock starts when the handler gets the parsed request...
        const auto t0 = std::chrono::steady_clock::now();
        const std::string prefix = req.get_param_value("prefix");
        std::size_t limit = 5;
        if (req.has_param("limit")) {
            const long n = std::strtol(req.get_param_value("limit").c_str(), nullptr, 10);
            limit = n < 0 ? 0 : (n > 50 ? 50 : static_cast<std::size_t>(n));
        }
        const std::string engine = req.has_param("engine") ? req.get_param_value("engine") : "trie";
        std::string body =
            AutocorrectService::suggestionsJson(service.suggest(prefix, limit, engine), engine);
        // ...and stops once the lookup has run and the JSON is built. The time is then
        // appended to the body (a trivial string append, not part of the measurement).
        const auto micros =
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - t0)
                .count();
        body.pop_back();  // drop the closing '}'
        body += ",\"micros\":" + std::to_string(micros) + "}";
        res.set_content(body, "application/json");
    });

    svr.Post("/api/accept", [&](const httplib::Request& req, httplib::Response& res) {
        const bool ok = service.accept(req.get_param_value("word"));
        res.set_content(ok ? "{\"ok\":true}" : "{\"ok\":false}", "application/json");
    });

    if (!staticDir.empty() && !svr.set_mount_point("/", staticDir)) {
        std::cerr << "Could not serve static files from " << staticDir << '\n';
        return 1;
    }

    std::cout << "Listening on http://localhost:" << port << '\n';
    if (!svr.listen("0.0.0.0", port)) {
        std::cerr << "Could not listen on port " << port << '\n';
        return 1;
    }
    return 0;
}
