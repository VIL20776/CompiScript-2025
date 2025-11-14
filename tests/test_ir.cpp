#include <catch2/catch_test_macros.hpp>

#include <print>

#include "test.h"

using namespace CompiScript;


TEST_CASE("Aritmetic and logic operations generation", "[Operation gen]") {
    auto generated_tac = test_ir_gen(R"(
let x = 5 + 3 * 2;
let y = !(x < 10 || x > 20);
let z = (1 + 2) * 3;
                )");

    std::string expected = R"(t0 = * 3 2
t1 = + 5 t0
W0_x =  t1 
t0 = < W0_x 10
t1 = > W0_x 20
t2 = || t0 t1
t3 = ! t2 
B0_y =  t3 
t0 = + 1 2
t1 = * t0 3
W0_z =  t1 
)";

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_tac.erase(remove(generated_tac.begin(), generated_tac.end(), ' '), generated_tac.end());

    REQUIRE(expected == generated_tac);

}

TEST_CASE("Function code generation", "[Function gen]") {
    auto generated_tac = test_ir_gen(R"(
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

    std::string expected = R"(begin F0_saludar 
arg S1_nombre
t0 = concat "Hola " S1_nombre
return t0 
end F0_saludar 
param "Mundo" 
call F0_saludar 
S0_mensaje =  ret 
begin F0_crearContador 
begin F2_siguiente 
return 1 
end F2_siguiente 
call F2_siguiente 
return ret 
end F0_crearContador 
)";

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_tac.erase(remove(generated_tac.begin(), generated_tac.end(), ' '), generated_tac.end());

    REQUIRE(expected == generated_tac);
    
}

TEST_CASE("Function with recursion code generation", "[Function gen]") {
    auto generated_tac = test_ir_gen(R"(
function factorial(n: integer): integer {
  if (n <= 1) { return 1; }
  return n * factorial(n - 1);
}
                )");

    std::string expected = R"(begin F0_factorial 
arg  W1_n
t0 = <= W1_n 1
if t0 l0
goto l1
tag l0
return 1 
tag l1
t0 = - W1_n 1
push W1_n
param t0
call F0_factorial
pop W1_n
t1 = * W1_n ret
return t1
end F0_factorial 
)";

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_tac.erase(remove(generated_tac.begin(), generated_tac.end(), ' '), generated_tac.end());

    REQUIRE(expected == generated_tac);
    
}

TEST_CASE("Array code generation", "[Array gen]") {
    auto generated_tac = test_ir_gen(R"(
let lista = [1, 2, 3];
print(lista[0]);
let matriz = [[1, 2], [3, 4]];
let num2 = matriz[0][1];
                )");
    std::string expected = R"(S0_lista = alloc 12 
        i = + S0_lista 0
        i* =  1 
        i = + S0_lista 4
        i* =  2 
        i = + S0_lista 8
        i* =  3 
        t0 = 0 
        err = >= t0 3
        iferr BAD_INDEX
        t0 = * t0 4 
        i = + S0_lista t0       
        p = to_str i* 4
        print
        S0_matriz = alloc 16 
        i = + S0_matriz 0
        i* =  1 
        i = + S0_matriz 4
        i* =  2 
        i = + S0_matriz 8
        i* =  3 
        i = + S0_matriz 12
        i* =  4 
        t0 =  0
        err = >= t0 2
        iferr BAD_INDEX 
        t0 = * t0 2
        t0 = * t0 4
        i = + S0_matriz t0
        t0 =  1
        err = >= t0 2
        iferr BAD_INDEX 
        t0 = * t0 4
        i = + i t0 
        W0_num2 =  i* 
    )";

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_tac.erase(remove(generated_tac.begin(), generated_tac.end(), ' '), generated_tac.end());

    REQUIRE(expected == generated_tac);
}

TEST_CASE("Class code generation", "[Class gen]") {
    auto generated_tac = test_ir_gen(R"(
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
    std::string expected = R"(begin F1_constructor 
        arg S2_this
        arg S2_nombre
        i = + S2_this 0
        i* = S2_nombre
        end F1_constructor
        begin F1_hablar  
        arg S3_this  
        i = + S3_this 0 
        t0 = concat i* " hace ruido." 
        return t0
        end F1_hablar
        t0 = alloc 4 
        param t0  
        param "Firulais"
        call F1_constructor 
        S0_animal = t0
        param S0_animal
        call F1_hablar
        p = ret
        print
        begin F4_hablar
        arg S5_this 
        i = + S5_this 0
        t0 = concat i* " ladra."
        return t0
        end F4_hablar
        t0 = alloc 4 
        param t0  
        param "Firulais"
        call F1_constructor 
        S0_perro = t0
        param S0_perro
        call F4_hablar
        p = ret
        print
    )";

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_tac.erase(remove(generated_tac.begin(), generated_tac.end(), ' '), generated_tac.end());

    REQUIRE(expected == generated_tac);
}

TEST_CASE("Conditionals code generation", "[Conditional gen]") {
    auto generated_tac = test_ir_gen(R"(
let x = 4;
if (x > 10) {
  print("Mayor a 10");
} else {
  print("Menor o igual");
}

while (x < 5) {
  x = x + 1;
}

do {
  x = x - 1;
} while (x > 0);
                )");
    std::string expected = R"(W0_x = 4
        t0 = > W0_x 10
        if t0 l0
        goto l1
        tag l0
        p = "Mayor a 10"
        print
        tag l1
        p = "Menor o igual"
        print
        tag l2
        t0 = < W0_x 5
        ifnot t0 l3
        t0 = + W0_x 1
        W0_x = t0
        goto l2
        tag l3
        tag l4
        t0 = - W0_x 1
        W0_x = t0
        t0 = > W0_x 0
        if t0 l4
        tag l5
    )";

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_tac.erase(remove(generated_tac.begin(), generated_tac.end(), ' '), generated_tac.end());

    REQUIRE(expected == generated_tac);
}

TEST_CASE("For loop code generation", "[For loop gen]") {
    auto generated_tac = test_ir_gen(R"(
for (let i: integer = 0; i < 3; i = i + 1) {
  print(i);
}

let notas = [40, 60, 80, 100];
foreach (n in notas) {
  if (n < 60) { continue; }
  if (n == 100) { break; }
  print(n);
}
                )");
    std::string expected = R"(W0_i = 0
        tag l0
        t0 = < W0_i 3
        ifnot t0 l1
        p = to_str W0_i 4
        print
        t0 = + W0_i 1
        W0_i = t0
        goto l0
        tag l1
        S0_notas = alloc 16
        i = + S0_notas 0
        i* = 40
        i = + S0_notas 4
        i* = 60
        i = + S0_notas 8
        i* = 80
        i = + S0_notas 12
        i* = 100
        i = S0_notas
        tag l2   
        W0_n = i*
        t0 = < W0_n 60
        if t0 l4  
        goto l5  
        tag l4   
        goto l2  
        tag l5   
        t0 = == W0_n 100
        if t0 l6  
        goto l7  
        tag l6   
        goto l3  
        tag l7   
        p = to_str W0_n 4
        print   
        i = + S0_notas 4
        t0 = + S0_notas 16
        t0 = < i t0
        if t0 l2  
        tag l3
    )";

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_tac.erase(remove(generated_tac.begin(), generated_tac.end(), ' '), generated_tac.end());

    REQUIRE(expected == generated_tac);
}

TEST_CASE("Try-Catch code generation", "[Error gen]") {
    auto generated_tac = test_ir_gen(R"(
let lista = [1, 2, 3, 4];
try {
  let peligro = lista[100];
} catch (err) {
  print("Error atrapado: " + err);
}
                )");
    std::string expected = R"(S0_lista = alloc 16
        i = + S0_lista 0
        i* = 1
        i = + S0_lista 4
        i* = 2
        i = + S0_lista 8
        i* = 3
        i = + S0_lista 12
        i* = 4
        catch = l0
        t0=100 
        err = >= t0 4 
        iferr BAD_INDEX 
        t0 = * t0 4  
        i = + S0_lista t0 
        W1_peligro = i* 
        catch = 0  
        begin l0 
        S0_err = err 
        t0 = concat "Error atrapado:" S0_err 
        p = t0  
        print 
        end l0
    )";

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_tac.erase(remove(generated_tac.begin(), generated_tac.end(), ' '), generated_tac.end());

    REQUIRE(expected == generated_tac);
}

TEST_CASE("Switch-Case code generation", "[Switch gen]") {
    auto generated_tac = test_ir_gen(R"(
let x = 2;
switch (x) {
  case 1:
    print("uno");
  case 2:
    print("dos");
  default:
    print("otro");
}
                )");
    std::string expected = R"(W0_x = 2
        switch = W0_x
        case = == switch 1
        ifnot case l0
        p = "uno"
        print
        goto l2  
        tag l0
        case = == switch 2
        ifnot case l1
        p = "dos"
        print
        goto l2 
        tag l1
        p = "otro"
        print
        tag l2
    )";

    expected.erase(remove(expected.begin(), expected.end(), ' '), expected.end());
    expected.erase(remove(expected.begin(), expected.end(), '\t'), expected.end());
    generated_tac.erase(remove(generated_tac.begin(), generated_tac.end(), ' '), generated_tac.end());

    REQUIRE(expected == generated_tac);
}
