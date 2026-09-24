#include "chad/core/error.h"

namespace chad {

Error errorAt(int line, const std::string& kind, const std::string& detail) {
    std::string message = "line " + std::to_string(line) + ": " + kind;
    if (!detail.empty()) {
        message += " -- " + detail;
    }
    return Error(message);
}

}
