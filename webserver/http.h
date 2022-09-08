#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

namespace http {
    struct request {
        std::string method;
        std::string path;
        std::string version;

        std::vector<std::pair<std::string, std::string>> raw_headers;
        std::unordered_map<std::string, std::vector<std::string>> headers;

        std::stringbuf body;
    };

    struct response {
        std::string version;
        std::string code;
        std::string message;

        std::unordered_map<std::string, std::vector<std::string>> headers;

        std::string body;
    };

    request parse(std::stringstream in);

    void write(const response& res, std::stringstream& out);
}
