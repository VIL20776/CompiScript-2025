#include <catch2/catch_test_macros.hpp>

#include <print>
#include <stdexcept>
#include <vector>
#include <string>

#include "SemanticChecker.h"
#include "SymbolTable.h"

#include "test.h"

using namespace CompiScript;

void checkErrors(std::vector<std::string> test_strings, const std::string expected_msg) { 
        SemanticChecker checker {};

        for (auto t: test_strings) {
            std::string error_msg {};
            try {
                test_stream(t, &checker);
            } catch (const std::runtime_error& error) {
                error_msg = error.what();
            }
            REQUIRE(error_msg == expected_msg);
        }
}

TEST_CASE("Type checking", "[NON_MATCHING_TYPES]") {
    std::string expect = "NON_MATCHING_TYPES";

    SECTION("Aritmetic and logical operations") {
        std::vector<std::string> test_strings {
            "5 + 3 * true;",
            "false - 1;",
            "true > 2;",
            R"(5 || "hola";)",
            R"(5 && "hola";)",
            R"(5 != "hola";)",
            "!2;",
            "[4, true];",
        };
        checkErrors(test_strings, expect);
    }

    SECTION("Assignments") {
        std::vector<std::string> test_strings {
            "let x: integer = true;",
            "const x: string = 4;",
            R"(
            let x = "hola";
            x = true;
            )",
        };
        checkErrors(test_strings, expect);
    }

    SECTION("Classes and objects") {
        std::vector<std::string> test_strings {
            R"(
            class Animal {
                let nombre: string;

                function constructor(nombre: string) {
                    this.nombre = nombre;
                }
            }

            let animal: Animal = new Animal();
            animal.nombre = 4;
            )",
            "animal = new Animal(4);",
        };
        checkErrors(test_strings, expect);
    }

    SECTION("Functions") {
        std::vector<std::string> test_strings {
            R"(
            function saludar(nombre: string): string {
                return "Hola " + nombre;
            }

            saludar(true);
            )",
        };
        checkErrors(test_strings, expect);
    }

}

TEST_CASE("Type validation", "[INVALID_TYPE]") {
    std::string expect = "INVALID_TYPE";

    std::vector<std::string> test_strings {
        R"(
        class Animal {
            let nombre: string;
        }

        let animal: Animal = new Animal();
        print(animal);
        )",
        R"(if ("Hola") {})",
        R"(while (4) {})",
        R"(do {} while (null);)",
        R"(for (; "hello";) {})",
    };
    checkErrors(test_strings, expect);
}
