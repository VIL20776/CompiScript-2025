#include <catch2/catch_test_macros.hpp>

#include <print>

#include "SemanticChecker.h"

#include "test.h"

using namespace CompiScript;


TEST_CASE("Aritmetic and logic operations generation", "[Operation gen]") {
    SemanticChecker checker {};
    auto generated_tac = test_ir_gen(R"(
let x = 5 + 3 * 2;
let y = !(x < 10 || x > 20);
let z = (1 + 2) * 3;
                )", &checker);

    REQUIRE(R"(t0 = * 3 2
t1 = + 5 t0
L0_x =  t1 
t0 = < L0_x 10
t1 = > L0_x 20
t2 = || t0 t1
t3 = ! t2 
L0_y =  t3 
t0 = + 1 2
t1 = * t0 3
L0_z =  t1 
)" == generated_tac);

}
