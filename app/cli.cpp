#include "cli.h"

bool isOption(const std::string& argument) {
    return argument.size() > 1 && argument[0] == '-';
}

const char* usageText() {
    return "usage: chad <file.chad>\n"
           "\n"
           "  -v, --version  print the version\n"
           "  -h, --help     print this\n";
}

const char* versionText() {
    return "chad " CHAD_VERSION;
}

Arguments parseArguments(int argc, char** argv) {
    Arguments parsed;

    if (argc < 2) {
        parsed.ok = false;
        parsed.error = "no input file. usage: chad <file.chad>";
    } else if (argc == 2) {
        const char* arg = argv[1];
        if (std::string(arg) == "-v" || std::string(arg) == "--version") {
            parsed.out = versionText();
        } else if (std::string(arg) == "-h" || std::string(arg) == "--help") {
            parsed.out = usageText();
        } else if (isOption(arg)) {
            parsed.ok = false;
            parsed.error = "unknown option '" + std::string(arg) + "'. use -h for help";
        } else {
            parsed.path = arg;
        }
    } else {
        parsed.ok = false;
        parsed.error = "too many arguments. usage: chad <file.chad>";
    }

    return parsed;
}
