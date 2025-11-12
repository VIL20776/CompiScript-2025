#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <print>
#include <iostream>

#include "test.h"

using namespace CompiScript;


TEST_CASE("Aritmetic and logic operations asm generation", "[Operation asm]") {
    auto generated_tac = test_mips_gen(R"(
let x = 5 + 3 * 2;
let y = !(x < 10 || x > 20);
let z = (1 + 2) * 3;
                )");

    std::string expected = R"(.data 
W0_x:           .word   0 
B0_y:           .byte   0 
W0_z:           .word   0 
.text 
main: 
li $t0, 3 
li $t1, 2 
mult $t0, $t1 
mflo $t2 
li $t0, 5 
add $t1, $t0, $t2 
move $s0, $t1 
sw $s0, W0_x 
li $t0, 10 
slt $t1, $s0, $t0 
li $t0, 20 
sgt $t2, $s0, $t0 
or $t0, $t1, $t2 
not $t3, $t0 
move $s1, $t3 
sb $s1, B0_y 
li $t0, 1 
li $t1, 2 
add $t2, $t0, $t1 
li $t0, 3 
mult $t2, $t0 
mflo $t1 
move $s2, $t1 
sw $s2, W0_z
)";

    std::ofstream out("output.s", std::ofstream::out);
    out << generated_tac;
    out.close();

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_tac.erase(remove(generated_tac.begin(), generated_tac.end(), ' '), generated_tac.end());

    REQUIRE(expected == generated_tac);

}
