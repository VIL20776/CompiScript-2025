#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <print>
#include <iostream>

#include "test.h"

using namespace CompiScript;


TEST_CASE("Aritmetic and logic operations asm generation", "[Operation asm]") {
    auto generated_mips = test_mips_gen(R"(
let x = 5 + 3 * 2;
let y = !(x < 10 || x > 20);
let z = (1 + 2) * 3;
                )");

    std::string expected = R"(.data 
W0_x:       .word   0 
B0_y:       .byte   0 
W0_z:       .word   0 
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

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_mips.erase(remove(generated_mips.begin(), generated_mips.end(), ' '), generated_mips.end());
    generated_mips.erase(remove(generated_mips.begin(), generated_mips.end(), '\t'), generated_mips.end());

    REQUIRE(expected == generated_mips);
}

TEST_CASE("Function asm generation", "[Function asm]") {
    auto generated_mips = test_mips_gen(R"(
function saludar(nombre: string): string {
  return "Hola " + nombre;
}

let mensaje = saludar("Mundo");

function crearContador(): integer {
  function siguiente(): integer {
    return 1;
  }
  return siguiente();
}
                )");

    std::string expected = R"(.data
str0:		.asciiz	"Hola "
str1:		.asciiz	"Mundo"
S0_mensaje:		.word	0
.text
F2_siguiente:
li $t3, 1
move $v0, $t3
jr $ra

F0_crearContador:
addi $sp, -4
sw $ra, ($sp)
jal F2_siguiente
lw $ra, ($sp)
addi $sp, 4
move $v0, $v0
jr $ra

F0_saludar:
la $t0, str0
addi $sp, -4
sw $a0, ($sp)
addi $sp, -4
sw $a1, ($sp)
move $a0, $t0
lw $a1, 4($sp)
addi $sp, -4
sw $ra, ($sp)
# jal concat_strings
lw $ra, ($sp)
addi $sp, 4
lw $a1, ($sp)
addi $sp, 4
lw $a0, ($sp)
addi $sp, 4
move $t1, $v0
move $v0, $t1
jr $ra

main:
la $t2, str1
move $a1, $t2
addi $sp, -4
sw $ra, ($sp)
jal F0_saludar
lw $ra, ($sp)
addi $sp, 4
move $s0, $v0
sw $s0, S0_mensaje

)";

    std::ofstream out("output.s", std::ofstream::out);
    out << generated_mips;
    out.close();

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_mips.erase(remove(generated_mips.begin(), generated_mips.end(), ' '), generated_mips.end());
    generated_mips.erase(remove(generated_mips.begin(), generated_mips.end(), '\t'), generated_mips.end());

    REQUIRE(expected == generated_mips);
    
}

TEST_CASE("Function with recursion asm generation", "[Function asm]") {
    auto generated_mips = test_mips_gen(R"(
function factorial(n: integer): integer {
  if (n <= 1) { return 1; }
  return n * factorial(n - 1);
}
let fac = factorial(4);
                )");

    std::string expected = R"(.data
W0_fac:		.word	0
.text
F0_factorial:
li $t0, 1
sle $t1, $a0, $t0
bne $zero, $t1, l0
b l1
l0:
li $t0, 1
move $v0, $t0
jr $ra

l1:
sub $t1, $a0, $t0
addi $sp, -4
sw $a0, ($sp)
move $a0, $t1
addi $sp, -4
sw $ra, ($sp)
jal F0_factorial
lw $ra, ($sp)
addi $sp, 4
lw $a0, ($sp)
addi $sp, 4
mult $a0, $v0
mflo $t0
move $v0, $t0
jr $ra

main:
li $t2, 4
move $a0, $t2
addi $sp, -4
sw $ra, ($sp)
jal F0_factorial
lw $ra, ($sp)
addi $sp, 4
move $s0, $v0
sw $s0, W0_fac

)";

    // std::ofstream out("output.s", std::ofstream::out);
    // out << generated_mips;
    // out.close();

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_mips.erase(remove(generated_mips.begin(), generated_mips.end(), ' '), generated_mips.end());
    generated_mips.erase(remove(generated_mips.begin(), generated_mips.end(), '\t'), generated_mips.end());

    REQUIRE(expected == generated_mips);
}
