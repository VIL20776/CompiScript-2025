#include <catch2/catch_test_macros.hpp>

#include <print>

#include "CompiScriptLexer.h"
#include "CompiScriptParser.h"
#include "SemanticChecker.h"
#include "SymbolTable.h"

using namespace CompiScript;
void test_stream(const std::string &stream, SemanticChecker *checker) {
    auto input = antlr4::ANTLRInputStream(stream);
    CompiScriptLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    CompiScriptParser parser(&tokens);
    checker->visitProgram(parser.program());
}

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
        REQUIRE(d.value == "");

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
        REQUIRE(notas.size == 96);
        REQUIRE(notas.dimentions == 1);

        auto lista = table.lookup("lista").first;
        REQUIRE(table.lookup("lista").second);
        REQUIRE(lista.data_type == SymbolDataType::INTEGER);
        REQUIRE(lista.size == 96);
        REQUIRE(lista.dimentions == 1);

        auto matriz = table.lookup("matriz").first;
        REQUIRE(table.lookup("matriz").second);
        REQUIRE(matriz.data_type == SymbolDataType::INTEGER);
        REQUIRE(matriz.size == 128);
        REQUIRE(matriz.dimentions == 2);
    }

    SECTION("Checking array access") {
        auto nota = table.lookup("nota").first;
        REQUIRE(table.lookup("nota").second);
        REQUIRE(nota.data_type == SymbolDataType::INTEGER);
        REQUIRE(nota.dimentions == 0);
    }

}
