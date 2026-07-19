#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

struct Command {
    std::string usage;
    std::string description;
    std::function<void(const std::vector<std::string>&)> handler;
};

class CLI {
    std::map<std::string, Command> commands;
    std::string appName;

public:
    CLI(std::string name) : appName(std::move(name)) {}

    void addCommand(const std::string& name, const std::string& usage,
                     const std::string& description,
                     std::function<void(const std::vector<std::string>&)> handler) {
        commands[name] = {usage, description, std::move(handler)};
    }

    void printHelp(const std::string& specific = "") {
        if (!specific.empty() && commands.count(specific)) {
            auto& cmd = commands[specific];
            std::cout << "Usage: " << appName << " " << cmd.usage << "\n\n"
                      << cmd.description << "\n";
            return;
        }
        std::cout << "Usage: " << appName << " <command>\n\nCommands:\n";
        for (auto& [name, cmd] : commands) {
            std::cout << "  " << name;
            for (size_t i = name.size(); i < 12; i++) std::cout << " ";
            std::cout << cmd.description << "\n";
        }
        std::cout << "\nRun '" << appName << " help <command>' for details.\n";
    }

    void run(int argc, char* argv[]) {
        if (argc < 2) { printHelp(); return; }
        std::string cmdName = argv[1];
        std::vector<std::string> args(argv + 2, argv + argc);

        if (cmdName == "help") { printHelp(args.empty() ? "" : args[0]); return; }

        if (commands.count(cmdName)) {
            commands[cmdName].handler(args);
        } else {
            std::cout << "Unknown command: " << cmdName << "\n\n";
            printHelp();
        }
    }
};

// ---- helpers ----
std::string toLowerStr(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}
std::string toUpperStr(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

int main(int argc, char* argv[]) {
    CLI cli("filetool");

    // list <dir>
    cli.addCommand("list", "list <dir>", "List files in a directory",
        [](const std::vector<std::string>& args) {
            std::string dir = args.empty() ? "." : args[0];
            if (!fs::exists(dir)) { std::cout << "No such directory: " << dir << "\n"; return; }
            for (auto& entry : fs::directory_iterator(dir)) {
                std::cout << (entry.is_directory() ? "[DIR]  " : "[FILE] ")
                          << entry.path().filename().string() << "\n";
            }
        });

    // search <text> <file-or-dir> [-w] [-i]
    cli.addCommand("search", "search <text> <file-or-dir> [-w] [-i]",
        "Search for text in a file/dir. -w = whole word only, -i = case-insensitive",
        [](const std::vector<std::string>& args) {
            if (args.size() < 2) { std::cout << "Error: need <text> and <file-or-dir>\n"; return; }
            std::string target = args[0], path = args[1];
            bool wholeWord = false, ignoreCase = false;
            for (size_t i = 2; i < args.size(); i++) {
                if (args[i] == "-w") wholeWord = true;
                else if (args[i] == "-i") ignoreCase = true;
            }
            if (!fs::exists(path)) { std::cout << "No such file or directory: " << path << "\n"; return; }

            std::string cmpTarget = ignoreCase ? toLowerStr(target) : target;
            int matches = 0;

            auto isWordChar = [](char c) {
                return std::isalnum((unsigned char)c) || c == '_';
            };

            auto lineMatches = [&](const std::string& line) -> bool {
                std::string cmpLine = ignoreCase ? toLowerStr(line) : line;
                size_t pos = 0;
                while ((pos = cmpLine.find(cmpTarget, pos)) != std::string::npos) {
                    if (!wholeWord) return true;
                    bool leftOk = (pos == 0) || !isWordChar(cmpLine[pos - 1]);
                    size_t endPos = pos + cmpTarget.size();
                    bool rightOk = (endPos >= cmpLine.size()) || !isWordChar(cmpLine[endPos]);
                    if (leftOk && rightOk) return true;
                    pos++;
                }
                return false;
            };

            auto searchFile = [&](const fs::path& p) {
                std::ifstream file(p);
                std::string line;
                int lineNum = 0;
                while (std::getline(file, line)) {
                    lineNum++;
                    if (lineMatches(line)) {
                        std::cout << p.string() << ":" << lineNum << ": " << line << "\n";
                        matches++;
                    }
                }
            };

            if (fs::is_regular_file(path)) {
                searchFile(path);
            } else if (fs::is_directory(path)) {
                for (auto& entry : fs::recursive_directory_iterator(path)) {
                    if (entry.is_regular_file()) searchFile(entry.path());
                }
            }
            std::cout << matches << " match(es) found.\n";
        });

    // rename <dir> <find> <replace>
    cli.addCommand("rename", "rename <dir> <find> <replace>", "Batch rename files replacing substring in filename",
        [](const std::vector<std::string>& args) {
            if (args.size() < 3) { std::cout << "Error: need <dir> <find> <replace>\n"; return; }
            std::string dir = args[0], find = args[1], replace = args[2];
            if (!fs::exists(dir)) { std::cout << "No such directory: " << dir << "\n"; return; }
            int renamed = 0;
            for (auto& entry : fs::directory_iterator(dir)) {
                std::string name = entry.path().filename().string();
                size_t pos = name.find(find);
                if (pos != std::string::npos) {
                    std::string newName = name;
                    newName.replace(pos, find.length(), replace);
                    fs::path newPath = entry.path().parent_path() / newName;
                    fs::rename(entry.path(), newPath);
                    std::cout << name << " -> " << newName << "\n";
                    renamed++;
                }
            }
            std::cout << renamed << " file(s) renamed.\n";
        });

    // wc <file>
    cli.addCommand("wc", "wc <file>", "Count lines, words, and characters in a file",
        [](const std::vector<std::string>& args) {
            if (args.empty()) { std::cout << "Error: need <file>\n"; return; }
            std::ifstream file(args[0]);
            if (!file) { std::cout << "Cannot open file: " << args[0] << "\n"; return; }
            std::string line;
            long lines = 0, words = 0, chars = 0;
            while (std::getline(file, line)) {
                lines++;
                chars += line.size() + 1;
                std::istringstream iss(line);
                std::string w;
                while (iss >> w) words++;
            }
            std::cout << "lines: " << lines << "  words: " << words << "  chars: " << chars << "\n";
        });

    // convert <file> <upper|lower>
    cli.addCommand("convert", "convert <file> <upper|lower>", "Convert file text case in place",
        [](const std::vector<std::string>& args) {
            if (args.size() < 2) { std::cout << "Error: need <file> <upper|lower>\n"; return; }
            std::string path = args[0], mode = args[1];
            std::ifstream in(path);
            if (!in) { std::cout << "Cannot open file: " << path << "\n"; return; }
            std::stringstream buffer;
            buffer << in.rdbuf();
            in.close();
            std::string content = buffer.str();
            content = (mode == "upper") ? toUpperStr(content) : toLowerStr(content);
            std::ofstream out(path);
            out << content;
            std::cout << "Converted " << path << " to " << mode << "case.\n";
        });

    cli.run(argc, argv);
    return 0;
}