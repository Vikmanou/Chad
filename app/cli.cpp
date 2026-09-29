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

Arguments printing(const std::string& text) {
    Arguments parsed;
    parsed.out = text;
    return parsed;
}

Arguments failed(const std::string& error) {
    Arguments parsed;
    parsed.ok = false;
    parsed.error = error;
    return parsed;
}

Arguments parseArguments(int argc, char** argv) {
    Arguments parsed;

    for (int i = 1; i < argc; i++) {
        const std::string arg = argv[i];

        if (arg == "-v" || arg == "--version") {
            return printing(versionText());
        } else if (arg == "-h" || arg == "--help") {
            return printing(usageText());
        } else if (isOption(arg)) {
            return failed("unknown option `" + arg + "` -- use -h for help");
        } else if (parsed.path.empty()) {
            parsed.path = arg;
        } else {
            return failed("too many arguments -- usage: chad <file.chad>");
        }
    }

    if (parsed.path.empty()) return failed("no input file -- usage: chad <file.chad>");

    return parsed;
}