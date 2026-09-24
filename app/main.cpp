#include <iostream>
#include <optional>
#include <string>

#include "chad/core/file.h"
#include "cli.h"

int main(int argc, char** argv) {
    const Arguments parsed = parseArguments(argc, argv);

    if (parsed.ok) {
        if (!parsed.path.empty()) {
            std::cout << "File: " << parsed.path << std::endl;
        } else {
            std::cout << parsed.out << std::endl;
            return 0;
        }
    } else {
        std::cerr << "Error: " << parsed.error << std::endl;
        return 1;
    }

    const std::optional<std::string> source = chad::readFile(parsed.path);
    if (!source) {
        std::cerr << "chad: cannot read `" << parsed.path << "`\n";
        return 1;
    }

    std::cout << *source << std::endl;

    return 0;
}