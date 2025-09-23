#include <catch2/catch_test_macros.hpp>

#include <print>

#include "SemanticChecker.h"

#include "test.h"

using namespace CompiScript;


TEST_CASE("Aritmetic and logic operations generation", "[Operation gen]") {
    auto generated_tac = test_ir_gen(R"(
let x = 5 + 3 * 2;
let y = !(x < 10 || x > 20);
let z = (1 + 2) * 3;
                )");

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

    REQUIRE(R"(begin L0_saludar 
L1_nombre = param  
t0 = concat "Hola " L1_nombre
return t0 
end L0_saludar 
push "Mundo" 
ret = call L0_saludar 
L0_mensaje =  ret 
begin L0_crearContador 
begin L2_siguiente 
return 1 
end L2_siguiente 
ret = call L2_siguiente 
return ret 
end L0_crearContador 
)" == generated_tac);
    
}

TEST_CASE("Array code generation", "[Array gen]") {
    auto generated_tac = test_ir_gen(R"(
let lista = [1, 2, 3];
                )");

    REQUIRE(R"(L0_lista = alloc 12 
i = + L0_lista 0
i* =  1 
i = + L0_lista 4
i* =  2 
i = + L0_lista 8
i* =  3 
)" == generated_tac);
}
