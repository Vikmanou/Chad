#pragma once

#include <string>

struct Options {
    std::string path;
    bool wantsHelp = false;
    bool wantsVersion = false;
};

struct Arguments {
    std::string path;
    bool ok = true;
    std::string error;
    std::string out;
};

Arguments parseArguments(int argc, char** argv);

const char* usageText();

const char* versionText();