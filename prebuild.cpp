#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include <cstdio>
#include <dirent.h>
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
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir ("python")) != NULL) 
    {
        while ((ent = readdir (dir)) != NULL) 
        {
            std::string filename = ent->d_name;
            size_t pos = filename.rfind(".py");
            if (pos != std::string::npos)
            {
                std::string key = filename.substr(0, filename.length() - 3);
                std::string filepath = "python/" + filename;
                FILE* file = fopen(filepath.c_str(), "r");
                if(file == NULL) exit(2);
                std::string content;
                char buffer[1024];
                while (fgets(buffer, sizeof(buffer), file) != NULL)
                {
                    content += buffer;
                }
                fclose(file);
                sources[key] = to_hex_string(content);
            }
        }
        closedir (dir);
    }else{
        exit(1);
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

    for (auto it=sources.begin(); it!=sources.end(); ++it) {
        header += "        {\"" + it->first + "\", \"" + it->second + "\"},\n";
    }

    header += "    };\n";
    header += "} // namespace pkpy\n";

    return header;
}

int main() {
    auto sources = generate_python_sources();
    std::string header = generate_header(sources);
    FILE* f = fopen("src/_generated.h", "w");
    fprintf(f, "%s", header.c_str());
    fclose(f);
    return 0;
}