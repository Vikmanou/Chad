#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

#if !defined(_WIN32)
#include <sys/wait.h>
#endif

#include "chad/core/file.h"

namespace fs = std::filesystem;

std::string read(const fs::path& path) {
    return chad::readFile(path.string()).value_or("");
}

std::string quoted(const fs::path& path) {
    return "\"" + path.string() + "\"";
}

fs::path sibling(fs::path test, const char* extension) {
    return test.replace_extension(extension);
}

std::string check(const fs::path& test) {
    const fs::path temp = CHAD_TEMP_DIR;
    const fs::path input = temp / "in", output = temp / "out", error = temp / "err";
    std::ofstream(input, std::ios::binary) << read(sibling(test, ".stdin"));

    std::string command = quoted(CHAD_EXE) + " " + quoted(test) + " < " + quoted(input) + " > " + quoted(output) + " 2> " + quoted(error);

#if defined(_WIN32)
    const int exitCode = std::system(("\"" + command + "\"").c_str());
#else
    const int exitCode = WEXITSTATUS(std::system(command.c_str()));
#endif

    const bool wantsError = fs::exists(sibling(test, ".stderr"));
    const std::string wantedOutput = read(sibling(test, ".stdout"));
    const std::string wantedError = read(sibling(test, ".stderr"));

    if (exitCode != (wantsError ? 1 : 0)) {
        return "exit code " + std::to_string(exitCode) + " " + read(error);
    } else if (read(output) != wantedOutput) {
        return "output `" + read(output) + "`, wanted `" + wantedOutput + "`";
    } else if (read(error).find(wantedError) == std::string::npos) {
        return "error `" + read(error) + "`, wanted `" + wantedError + "`";
    }
    
    return "";
}

int main() {
    std::set<fs::path> tests;
    for (const auto& entry : fs::directory_iterator(fs::path(CHAD_ROOT_DIR) / "tests" / "cases")) {
        if (entry.path().extension() == ".chad") {
            tests.insert(entry.path());
        }
    }

    int failed = 0;
    for (const fs::path& test : tests) {
        const std::string failure = check(test);
        if (!failure.empty()) {
            failed++;
            std::cout << "FAIL " << test.stem().string() << ": " << failure << "\n";
        }
    }

    std::cout << tests.size() - failed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
