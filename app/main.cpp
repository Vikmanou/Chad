#include <iostream>
#include <optional>
#include <string>

#ifdef _WIN32
    #include <fcntl.h>
    #include <io.h>
#endif

#include "chad/core/file.h"
#include "chad/core/error.h"
#include "chad/compiler/program.h"
#include "cli.h"
#include "chad/runtime/interpreter.h"

namespace {
// take a wild guess who is responsible for this... Windows.
void useBinaryStreams() {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif
}

}

int main(int argc, char** argv) {
    const Arguments parsed = parseArguments(argc, argv);

    if (parsed.ok) {
        if (parsed.path.empty()) {
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

    useBinaryStreams();

    try {
        const chad::Program program = chad::compile(*source);
        chad::run(program);
    } catch (const chad::Error& error) {
        std::fflush(stdout);
        std::cerr << error.what() << "\n";
        return 1;
    }

    return 0;
}