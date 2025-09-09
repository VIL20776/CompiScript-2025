#include <catch2/catch_test_macros.hpp>

#include <print>

#include "SemanticChecker.h"

#include "test.h"

using namespace CompiScript;


TEST_CASE("Aritmetic and logic operations generation", "[Operation gen]") {
    SemanticChecker checker {};
    auto generated_tac = test_ir_gen(R"(
let x = 5 + 3 * 2;
// let y = !(x < 10 || x > 20);
let z = (1 + 2) * 3;
                )", &checker);

    std::print("{}", generated_tac.c_str());

}
