#include <catch2/catch_test_macros.hpp>

#include <print>

#include "SemanticChecker.h"
#include "SymbolTable.h"

#include "test.h"

using namespace CompiScript;

TEST_CASE("Variable testing", "[Data types]") {
    SemanticChecker checker {};
    test_stream(R"(
let a: integer = 10;
let b: string = "hola";
let c: boolean = true;
let d = null;

let nombre: string;
nombre = "Compiscript";
                )", &checker);

    auto table = checker.getSymbolTable();
    SECTION("Checking variable declaration") {
        REQUIRE(table.lookup("a").second);
        REQUIRE(table.lookup("b").second);
        REQUIRE(table.lookup("c").second);
        REQUIRE(table.lookup("d").second);
        REQUIRE(table.lookup("nombre").second);
    }
    
    SECTION("Checking variable data") {
        auto a = table.lookup("a").first;
        REQUIRE(a.data_type == SymbolDataType::INTEGER);
        REQUIRE(a.type == SymbolType::VARIABLE);
        REQUIRE(a.value == "10");

        auto b = table.lookup("b").first;
        REQUIRE(b.data_type == SymbolDataType::STRING);
        REQUIRE(b.value == "\"hola\"");

        auto c = table.lookup("c").first;
        REQUIRE(c.data_type == SymbolDataType::BOOLEAN);
        REQUIRE(c.value == "true");

        auto d = table.lookup("d").first;
        REQUIRE(d.data_type == SymbolDataType::NIL);
        REQUIRE(d.value == "null");

        auto nombre = table.lookup("nombre").first;
        REQUIRE(nombre.data_type == SymbolDataType::STRING);
        REQUIRE(nombre.value == "\"Compiscript\"");
    }
}

TEST_CASE("Aritmetic and logic operations", "[Operations]") {
    SemanticChecker checker {};
    test_stream(R"(
let x = 5 + 3 * 2;
let y = !(x < 10 || x > 20);
let z = (1 + 2) * 3;
                )", &checker);

    auto table = checker.getSymbolTable();

    SECTION("Checking variable types") {
        auto x = table.lookup("x").first;
        REQUIRE(x.data_type == SymbolDataType::INTEGER);

        auto y = table.lookup("y").first;
        REQUIRE(y.data_type == SymbolDataType::BOOLEAN);

        auto z = table.lookup("z").first;
        REQUIRE(z.data_type == SymbolDataType::INTEGER);
    }
}

TEST_CASE("Constants", "[Constants]") {
    SemanticChecker checker {};
    test_stream(R"(
const KiB: integer = 1024;
                )", &checker);

    auto table = checker.getSymbolTable();

    SECTION("Checking constant declaration") {
        auto kib = table.lookup("KiB").first;
        REQUIRE(kib.data_type == SymbolDataType::INTEGER);
    }
}

TEST_CASE("Functions", "[Functions]") {
    SemanticChecker checker {};
    test_stream(R"(
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
                )", &checker);

    auto table = checker.getSymbolTable();

    SECTION("Checking function declaration") {
        auto saludar = table.lookup("saludar").first;
        REQUIRE(table.lookup("saludar").second);
        REQUIRE(saludar.type == SymbolType::FUNCTION);
        REQUIRE(saludar.data_type == SymbolDataType::STRING);
    }

    SECTION("Checking function call") {
        auto mensaje = table.lookup("mensaje").first;
        REQUIRE(table.lookup("mensaje").second);
        REQUIRE(mensaje.data_type == SymbolDataType::STRING);
    }

    SECTION("Checking function closure") {
        auto contador = table.lookup("crearContador").first;
        REQUIRE(table.lookup("crearContador").second);
        REQUIRE(contador.definition.lock()->table.contains("siguiente"));
    } 
}

TEST_CASE("Recursion", "[Functions]") {
    SemanticChecker checker {};
    test_stream(R"(
function factorial(n: integer): integer {
  if (n <= 1) { return 1; }
  return n * factorial(n - 1);
}
                )", &checker);

    auto table = checker.getSymbolTable();
    SECTION("Function declaration with recursion") {
        REQUIRE(table.lookup("factorial").second);
    }

}

TEST_CASE("Arrays", "[Arrays]") {
    SemanticChecker checker {};
    test_stream(R"(
let notas: integer[] = [90, 85, 100];
let lista = [1, 2, 3];
let matriz: integer[][] = [[1, 2], [3, 4]];

let nota = notas[0];
                )", &checker);

    auto table = checker.getSymbolTable();

    SECTION("Checking array declaration") {
        auto notas = table.lookup("notas").first;
        REQUIRE(table.lookup("notas").second);
        REQUIRE(notas.data_type == SymbolDataType::INTEGER);
        REQUIRE(notas.size == 12);
        REQUIRE(notas.dimentions.size() == 1);

        auto lista = table.lookup("lista").first;
        REQUIRE(table.lookup("lista").second);
        REQUIRE(lista.data_type == SymbolDataType::INTEGER);
        REQUIRE(lista.size == 12);
        REQUIRE(lista.dimentions.size() == 1);

        auto matriz = table.lookup("matriz").first;
        REQUIRE(table.lookup("matriz").second);
        REQUIRE(matriz.data_type == SymbolDataType::INTEGER);
        REQUIRE(matriz.size == 16);
        REQUIRE(matriz.dimentions.size() == 2);
    }

    SECTION("Checking array access") {
        auto nota = table.lookup("nota").first;
        REQUIRE(table.lookup("nota").second);
        REQUIRE(nota.data_type == SymbolDataType::INTEGER);
        REQUIRE(nota.dimentions.size() == 0);
    }

}

TEST_CASE("Class tests", "[Classes]") {
    SemanticChecker checker {};
    test_stream(R"(
class Animal {
  let nombre: string;

  function constructor(nombre: string) {
    this.nombre = nombre;
  }

  function hablar(): string {
    return this.nombre + " hace ruido.";
  }
}

let animal: Animal = new Animal("Toby");

class Perro : Animal {
  function hablar(): string {
    return this.nombre + " ladra.";
  }
}

let perro: Perro = new Perro("Firulais");
                )", &checker);

    auto table = checker.getSymbolTable();
    SECTION("Class declaration") {
        auto animal = table.lookup("Animal").first;
        REQUIRE(table.lookup("Animal").second);
        REQUIRE(!animal.arg_list.empty());
    }

    SECTION("Class declaration with inheritance") {
        auto perro = table.lookup("Perro").first;
        REQUIRE(table.lookup("Perro").second);
        REQUIRE(perro.parent == "Animal");
        REQUIRE(!perro.arg_list.empty());
    }

    SECTION("Object declaration") {
        auto animal = table.lookup("animal").first;
        REQUIRE(table.lookup("animal").second);
        REQUIRE(animal.parent == "Animal");
        REQUIRE(table.get_property(animal.parent, "nombre").second);

        auto perro = table.lookup("perro").first;
        REQUIRE(table.lookup("perro").second);
        REQUIRE(perro.parent == "Perro");
        REQUIRE(table.get_property(perro.parent, "hablar").second);
        REQUIRE(table.get_property(perro.parent, "nombre").second);

    }
}

TEST_CASE("Conditional control", "[Flow]") {
    SemanticChecker checker {};
    test_stream(R"(

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
                )", &checker);
}

TEST_CASE("For loops", "[Flow]") {
    SemanticChecker checker {};
    test_stream(R"(
for (let i: integer = 0; i < 3; i = i + 1) {
  print(i);
}

let notas = [40, 60, 80, 100];
foreach (n in notas) {
  if (n < 60) { continue; }
  if (n == 100) { break; }
  print(n);
}
                )", &checker);
}

TEST_CASE("Switch case", "[Flow]") {
    SemanticChecker checker {};
    test_stream(R"(
let x = 2;
switch (x) {
  case 1:
    print("uno");
  case 2:
    print("dos");
  default:
    print("otro");
}
                )", &checker);
}

TEST_CASE("Try-Catch", "[Errors]") {
    SemanticChecker checker {};
    test_stream(R"(
let lista = [1, 2, 3, 4];
try {
  let peligro = lista[100];
} catch (err) {
  print("Error atrapado: " + err);
}
                )", &checker);
}


