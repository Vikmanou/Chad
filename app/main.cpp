#include <iostream>
#include <optional>
#include <string>

#include "cli.h"

int main(int argc, char** argv) {
    const Arguments parsed = parseArguments(argc, argv);

    if (parsed.ok) {
        if (!parsed.path.empty()) {
            std::cout << "File: " << parsed.path << std::endl;
        } else {
            std::cout << parsed.out << std::endl;
        }
    } else {
        std::cerr << "Error: " << parsed.error << std::endl;
        return 1;
    }

    return 0;
}