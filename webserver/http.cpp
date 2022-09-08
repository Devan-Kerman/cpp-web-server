#include "http.h"

http::request http::parse(std::stringstream in) {
    std::string method, path, version;
    std::getline(in, method, ' ');
    std::getline(in, path, ' ');
    std::getline(in, version);

    std::vector<std::pair<std::string, std::string>> raw_headers;
    std::unordered_map<std::string, std::vector<std::string>> headers;

    std::string line;

    while (true) {
        std::getline(in, line);

        if (line.empty()) {
            break;
        }

        auto help = line.find(": ");

        if (help == std::string::npos) {
            continue;
        }

        raw_headers.emplace_back(std::make_pair(line.substr(0, help), line.substr(help + 2, line.length())));
    }

    size_t contentLength = 0;

    for (const auto& pair : raw_headers) {
        if (headers.find(pair.first) == headers.end()) {
            headers[pair.first] = std::vector<std::string> {pair.second};
        } else {
            headers[pair.first].emplace_back(pair.second);
        }

        if (pair.first == "Content-Length") {
            try {
                contentLength = std::stoi(pair.second);
            } catch (...) {
            }
        }
    }

    return {
        .method = method,
        .path = path,
        .version = version,
        .raw_headers = raw_headers,
        .headers = headers,
    };
}

void http::write(const http::response& res, std::stringstream& out) {
    out << res.version << " " << res.code << " " << res.message << std::endl;

    for (const auto& pair : res.headers) {
        for (const auto& value : pair.second) {
            out << pair.first << ": " << value << std::endl;
        }
    }

    out << "Content-Length: " << res.body.size() << std::endl << std::endl;
    out << res.body;
}
