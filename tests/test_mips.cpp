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

jr $ra

)";

    std::ofstream out("arit_output.s", std::ofstream::out);
    out << generated_mips;
    out.close();

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
concat_string: 
# Guardar el número en $t0
    move $t0, $a0

    # Caso especial: si el número es 0
    beq $t0, $zero, es_cero

    # Calcular cantidad de dígitos
    li $t1, 0              # contador de dígitos
    move $t2, $t0
contar_digitos:
    beq $t2, $zero, reservar_memoria
    div $t2, 10
    mflo $t2
    addi $t1, $t1, 1
    j contar_digitos

es_cero:
    li $t1, 1              # solo un dígito para '0'

reservar_memoria:
    addi $t1, $t1, 1       # +1 para '\0'
    li $v0, 9              # syscall sbrk
    move $a0, $t1          # tamaño en bytes
    syscall
    move $t3, $v0          # dirección base
    addi $t4, $t3, 0       # puntero destino

    # Si el número era 0, escribir '0' y terminar
    beq $t0, $zero, escribir_cero

    # Convertir número a ASCII (en orden inverso)
    move $t2, $t0
    addi $t4, $t4, $t1     # puntero al final
    addi $t4, $t4, -1      # retroceder para '\0'
    sb $zero, 0($t4)       # escribir terminador
    addi $t4, $t4, -1

convertir_loop:
    beq $t2, $zero, fin_conversion
    div $t2, 10
    mfhi $t5               # resto
    mflo $t2               # cociente
    addi $t5, $t5, 48      # convertir a ASCII
    sb $t5, 0($t4)
    addi $t4, $t4, -1
    j convertir_loop

fin_conversion:
    move $v0, $t3          # retorno: dirección base
    jr $ra

escribir_cero:
    li $t5, 48             # '0'
    sb $t5, 0($t3)
    sb $zero, 1($t3)
    move $v0, $t3
    jr $ra

F2_siguiente:
li $t0, 1
move $v0, $t0
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
jal concat_strings
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
la $t0, str1
move $a0, $t0
addi $sp, -4
sw $ra, ($sp)
jal F0_saludar
lw $ra, ($sp)
addi $sp, 4
move $s0, $v0
sw $s0, S0_mensaje

jr $ra

)";

    std::ofstream out("func_output.s", std::ofstream::out);
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
li $t0, 1
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
li $t0, 4
move $a0, $t0
addi $sp, -4
sw $ra, ($sp)
jal F0_factorial
lw $ra, ($sp)
addi $sp, 4
move $s0, $v0
sw $s0, W0_fac

jr $ra

)";

    std::ofstream out("rec_output.s", std::ofstream::out);
    out << generated_mips;
    out.close();

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_mips.erase(remove(generated_mips.begin(), generated_mips.end(), ' '), generated_mips.end());
    generated_mips.erase(remove(generated_mips.begin(), generated_mips.end(), '\t'), generated_mips.end());

    REQUIRE(expected == generated_mips);
}

TEST_CASE("Array asm generation", "[Array asm]") {
    auto generated_mips = test_mips_gen(R"(
let lista = [1, 2, 3];
print(lista[0]);
let matriz = [[1, 2], [3, 4]];
let num2 = matriz[0][1];
                )");
    std::string expected = R"(.data
S0_lista:		.space	12
err_bad_index_msg:     .asciiz "Out of bounds index was recieved"
S0_matriz:		.space	16
W0_num2:		.word	0
.text
to_string:
# Calcular longitud de cadena1
    move $t0, $a0
    li $t3, 0
len1_loop:
    lb $t1, 0($t0)
    beq $t1, $zero, len1_done
    addi $t3, $t3, 1
    addi $t0, $t0, 1
    j len1_loop
len1_done:

    # Calcular longitud de cadena2
    move $t0, $a1
    li $t4, 0
len2_loop:
    lb $t1, 0($t0)
    beq $t1, $zero, len2_done
    addi $t4, $t4, 1
    addi $t0, $t0, 1
    j len2_loop
len2_done:

    # Calcular tamaño total = len1 + len2 + 1 (para '\0')
    add $t5, $t3, $t4
    addi $t5, $t5, 1

    # Reservar memoria con sbrk
    li $v0, 9          # syscall sbrk
    move $a0, $t5      # tamaño en bytes
    syscall
    move $t6, $v0      # guardar dirección base en $t6 (retorno)

    # Copiar cadena1 al bloque
    move $t0, $a0      # puntero cadena1
    move $t2, $t6      # puntero destino
copy1_loop:
    lb $t1, 0($t0)
    sb $t1, 0($t2)
    beq $t1, $zero, copy2_start
    addi $t0, $t0, 1
    addi $t2, $t2, 1
    j copy1_loop

copy2_start:
    move $t0, $a1      # puntero cadena2
copy2_loop:
    lb $t1, 0($t0)
    sb $t1, 0($t2)
    beq $t1, $zero, done
    addi $t0, $t0, 1
    addi $t2, $t2, 1
    j copy2_loop

done:
    move $v0, $t6      # retorno: dirección del bloque concatenado
    jr $ra

main:
la $s0, S0_lista
li $t0, 0
add $t8, $s0, $t0
li $t0, 1
sw $t0, ($t8)
li $t0, 4
add $t8, $s0, $t0
li $t0, 2
sw $t0, ($t8)
li $t0, 8
add $t8, $s0, $t0
li $t0, 3
sw $t0, ($t8)
li $t0, 0
move $t1, $t0
li $t0, 3
sge $t8, $t1, $t0
beq $zero, $t8, no_err0
beq $zero, $t9, err_bad_index
la $t8, err_bad_index_msg
addi $sp, -4
sw $ra, ($sp)
la $ra, clean_err0
jr $t9
clean_err0:
la $ra, ($sp)
addi $sp, 4
no_err0:
move $t8, $zero
move $t9, $zero
li $t0, 4
mult $t1, $t0
mflo $t1
add $t8, $s0, $t1
li $t0, 4
addi $sp, -4
sw $a0, ($sp)
addi $sp, -4
sw $a1, ($sp)
lw $a0, ($t8)
move $a1, $t0
addi $sp, -4
sw $ra, ($sp)
jal to_string
lw $ra, ($sp)
addi $sp, 4
lw $a1, ($sp)
addi $sp, 4
lw $a0, ($sp)
addi $sp, 4
move $v1, $v0
addi $sp, -4
sw $a0, ($sp)
move $a0, $v1
li $v0, 4
syscall
lw $a0, ($sp)
addi $sp, 4
move $zero, $v0
move $zero, $v1
la $s1, S0_matriz
li $t0, 0
add $t8, $s1, $t0
li $t0, 1
sw $t0, ($t8)
li $t0, 4
add $t8, $s1, $t0
li $t0, 2
sw $t0, ($t8)
li $t0, 8
add $t8, $s1, $t0
li $t0, 3
sw $t0, ($t8)
li $t0, 12
add $t8, $s1, $t0
li $t0, 4
sw $t0, ($t8)
li $t0, 0
move $t1, $t0
li $t0, 2
sge $t8, $t1, $t0
beq $zero, $t8, no_err1
beq $zero, $t9, err_bad_index
la $t8, err_bad_index_msg
addi $sp, -4
sw $ra, ($sp)
la $ra, clean_err1
jr $t9
clean_err1:
la $ra, ($sp)
addi $sp, 4
no_err1:
move $t8, $zero
move $t9, $zero
li $t0, 2
mult $t1, $t0
mflo $t1
li $t0, 4
mult $t1, $t0
mflo $t1
add $t8, $s1, $t1
li $t0, 1
move $t1, $t0
li $t0, 2
sge $t8, $t1, $t0
beq $zero, $t8, no_err2
beq $zero, $t9, err_bad_index
la $t8, err_bad_index_msg
addi $sp, -4
sw $ra, ($sp)
la $ra, clean_err2
jr $t9
clean_err2:
la $ra, ($sp)
addi $sp, 4
no_err2:
move $t8, $zero
move $t9, $zero
li $t0, 4
mult $t1, $t0
mflo $t1
add $t8, $t8, $t1
lw $s2, ($t8)

jr $ra

)";

    std::ofstream out("array_output.s", std::ofstream::out);
    out << generated_mips;
    out.close();

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_mips.erase(remove(generated_mips.begin(), generated_mips.end(), ' '), generated_mips.end());
    generated_mips.erase(remove(generated_mips.begin(), generated_mips.end(), '\t'), generated_mips.end());

    REQUIRE(expected == generated_mips);
}

TEST_CASE("Class asm generation", "[Class asm]") {
    auto generated_mips = test_mips_gen(R"(
class Animal {
  let nombre: string;

  function constructor(nombre: string) {
    this.nombre = nombre;
  }

  function hablar(): string {
    return this.nombre + " hace ruido.";
  }
}

let animal = new Animal("Firulais");
print(animal.hablar());

class Perro: Animal {
    function hablar(): string {
        return this.nombre + " ladra.";
    }
}

let perro = new Perro("Firulais");
print(perro.hablar());
                )");

    std::string expected = R"(.data
str0:		.asciiz	" hace ruido."
str1:		.asciiz	"Firulais"
S0_animal:		.word	0
str2:		.asciiz	" ladra."
str3:		.asciiz	"Firulais"
S0_perro:		.word	0
.text
concat_string: 
# Guardar el número en $t0
    move $t0, $a0

    # Caso especial: si el número es 0
    beq $t0, $zero, es_cero

    # Calcular cantidad de dígitos
    li $t1, 0              # contador de dígitos
    move $t2, $t0
contar_digitos:
    beq $t2, $zero, reservar_memoria
    div $t2, 10
    mflo $t2
    addi $t1, $t1, 1
    j contar_digitos

es_cero:
    li $t1, 1              # solo un dígito para '0'

reservar_memoria:
    addi $t1, $t1, 1       # +1 para '\0'
    li $v0, 9              # syscall sbrk
    move $a0, $t1          # tamaño en bytes
    syscall
    move $t3, $v0          # dirección base
    addi $t4, $t3, 0       # puntero destino

    # Si el número era 0, escribir '0' y terminar
    beq $t0, $zero, escribir_cero

    # Convertir número a ASCII (en orden inverso)
    move $t2, $t0
    addi $t4, $t4, $t1     # puntero al final
    addi $t4, $t4, -1      # retroceder para '\0'
    sb $zero, 0($t4)       # escribir terminador
    addi $t4, $t4, -1

convertir_loop:
    beq $t2, $zero, fin_conversion
    div $t2, 10
    mfhi $t5               # resto
    mflo $t2               # cociente
    addi $t5, $t5, 48      # convertir a ASCII
    sb $t5, 0($t4)
    addi $t4, $t4, -1
    j convertir_loop

fin_conversion:
    move $v0, $t3          # retorno: dirección base
    jr $ra

escribir_cero:
    li $t5, 48             # '0'
    sb $t5, 0($t3)
    sb $zero, 1($t3)
    move $v0, $t3
    jr $ra

F4_hablar:
li $t0, 0
add $t8, $a0, $t0
la $t0, str2
addi $sp, -4
sw $a0, ($sp)
addi $sp, -4
sw $a1, ($sp)
move $a0, ($t8)
move $a1, $t0
addi $sp, -4
sw $ra, ($sp)
jal concat_strings
lw $ra, ($sp)
addi $sp, 4
lw $a1, ($sp)
addi $sp, 4
lw $a0, ($sp)
addi $sp, 4
move $t1, $v0
move $v0, $t1
jr $ra

F1_hablar:
li $t0, 0
add $t8, $a0, $t0
la $t0, str0
addi $sp, -4
sw $a0, ($sp)
addi $sp, -4
sw $a1, ($sp)
move $a0, ($t8)
move $a1, $t0
addi $sp, -4
sw $ra, ($sp)
jal concat_strings
lw $ra, ($sp)
addi $sp, 4
lw $a1, ($sp)
addi $sp, 4
lw $a0, ($sp)
addi $sp, 4
move $t1, $v0
move $v0, $t1
jr $ra

F1_constructor:
li $t0, 0
add $t8, $a0, $t0
sw $a1, ($t8)
jr $ra

main:
li $t0, 4
addi $sp, -4
sw $a0, ($sp)
move $a0, $t0
li $v0, 9
syscall
lw $a0, ($sp)
addi $sp, 4
li $v0, $t1
move $a0, $t1
la $t0, str1
move $a1, $t0
addi $sp, -4
sw $ra, ($sp)
jal F1_constructor
lw $ra, ($sp)
addi $sp, 4
move $s0, $t1
sw $s0, S0_animal
move $a0, $s0
addi $sp, -4
sw $ra, ($sp)
jal F1_hablar
lw $ra, ($sp)
addi $sp, 4
move $v1, $v0
addi $sp, -4
sw $a0, ($sp)
move $a0, $v1
li $v0, 4
syscall
lw $a0, ($sp)
addi $sp, 4
move $zero, $v0
move $zero, $v1
li $t0, 4
addi $sp, -4
sw $a0, ($sp)
move $a0, $t0
li $v0, 9
syscall
lw $a0, ($sp)
addi $sp, 4
li $v0, $t1
move $a0, $t1
la $t0, str3
move $a1, $t0
addi $sp, -4
sw $ra, ($sp)
jal F1_constructor
lw $ra, ($sp)
addi $sp, 4
move $s1, $t1
sw $s1, S0_perro
move $a0, $s1
addi $sp, -4
sw $ra, ($sp)
jal F4_hablar
lw $ra, ($sp)
addi $sp, 4
move $v1, $v0
addi $sp, -4
sw $a0, ($sp)
move $a0, $v1
li $v0, 4
syscall
lw $a0, ($sp)
addi $sp, 4
move $zero, $v0
move $zero, $v1

jr $ra

)";

    std::ofstream out("class_output.s", std::ofstream::out);
    out << generated_mips;
    out.close();

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_mips.erase(remove(generated_mips.begin(), generated_mips.end(), ' '), generated_mips.end());
    generated_mips.erase(remove(generated_mips.begin(), generated_mips.end(), '\t'), generated_mips.end());

    REQUIRE(expected == generated_mips);
}
