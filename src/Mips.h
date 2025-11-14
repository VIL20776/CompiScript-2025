#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <array>

#include "IRGenerator.h"

namespace CompiScript {

struct Register {
    std::string reg;
    std::string text;
};

class Mips {
    private:
    std::vector<Quad> quadruplets;
    std::string assembly;

    std::array<std::string, 8> temporaries;
    std::array<std::string, 8> saved;
    std::array<std::string, 4> args;

    std::unordered_multimap<std::string, std::string> variables;

    Register spill_or_assign(const std::string &var);
    Register getRegister(const std::string &var);

    public:
    Mips(const std::vector<Quad> &quadruplets);

    std::string generateDataSection();    
    std::string generateTextSection();

    std::string generateAssembly();
};

}
