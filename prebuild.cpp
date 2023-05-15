#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <ctime>

std::string to_hex_string(const std::string& input) {
    std::string result;
    result.reserve(input.length() * 4);
    for (const auto& c : input) {
        char buf[5] = {0};
        sprintf(buf, "%02x", static_cast<unsigned char>(c));
        result += "\\x";
        result += buf;
    }
    return result;
}

std::map<std::string, std::string> generate_python_sources() {
    std::map<std::string, std::string> sources;

    for (const auto& file : std::filesystem::directory_iterator("python")) {
        if (file.path().extension() == ".py") {
            std::string key = file.path().stem().string();
            std::ifstream f(file.path());
            std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            sources[key] = to_hex_string(content);
        }
    }

    return sources;
}

std::string generate_header(const std::map<std::string, std::string>& sources) {
    std::time_t timestamp = std::time(nullptr);
    std::tm* now = std::localtime(&timestamp);
    char timestamp_str[20];
    std::strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%d %H:%M:%S", now);

    std::string header;
    header += "#pragma once\n";
    header += "// generated on ";
    header += timestamp_str;
    header += "\n#include <map>\n#include <string>\n\nnamespace pkpy{\n";
    header += "    inline static std::map<std::string, std::string> kPythonLibs = {\n";

    for (const auto& [key, value] : sources) {
        header += "        {\"" + key + "\", \"" + value + "\"},\n";
    }

    header += "    };\n";
    header += "} // namespace pkpy\n";

    return header;
}

int main() {
    auto sources = generate_python_sources();
    std::string header = generate_header(sources);
    std::ofstream header_file("src/_generated.h");
    header_file << header;
    return 0;
}