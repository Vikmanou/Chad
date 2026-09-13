#pragma once

#include <string>
#include <vector>

namespace chad {

using Breed = int;

// the breeds every program has
namespace breed {
constexpr Breed Number = 0;
constexpr Breed World = 1;
constexpr Breed Chad = 2;
constexpr Breed Rep = 3;
constexpr Breed Ghost = 4;
constexpr Breed Say = 5;
constexpr Breed Read = 6;
constexpr Breed Eof = 7;
constexpr Breed Cons = 8;
constexpr Breed Nil = 9;
}

struct BreedInfo {
    std::string name;
    int valueCount = 0;
    int armCount = 0;
    int line = 0; // where it was first seen. 0 for built-ins
};

struct Program {
    std::vector<BreedInfo> breeds;
};

}