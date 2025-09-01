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
            "5 * true;",
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

            let animal: Animal = new Animal("Firu");
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
        R"(foreach (x in "hello") {})",
        R"(
        function saludar(): string {
            return 0;
        }
        )",
        R"(
        switch (animal) {}
        )",
        R"(
        let x = 4;
        switch (x) {
        case true:
            print("true");
        }
        )",
        R"(
        let y = 4;
        switch (x) {
        case y:
            print("true");
        }
        )",
        R"(
        let array = [1, 2, 3];
        array[true];
        )",
    };
    checkErrors(test_strings, expect);
}

TEST_CASE("Check redefinitions", "[REDEFINITION]") {
    std::string expect = "REDEFINITION";

    std::vector<std::string> test_strings {
        R"(
        class Animal {
            let nombre: string;
        }
        class Animal {
            let species: string;
        }
        )",
        R"(
        function saludar(): string {
            return "hola";
        }
        function saludar(): string {
            return "mundo";
        }
        )",
        R"(
        let x = 4;
        let x = "hola";
        )",
        R"(
        const y = 4;
        const y = "hola";
        )",
        R"(const Animal = "perro";)",
        R"(let saludar = "hola";)",
        R"(class x {})",
        R"(function y() {})",
    };
    checkErrors(test_strings, expect);
}

TEST_CASE("Check calls to undefined symbols", "[UNDEFINED_ACCESS]") {
    std::string expect = "UNDEFINED_ACCESS";

    std::vector<std::string> test_strings {
        R"(
        class Animal {
            let nombre: string;
        }
        let animal = new Animal();
        animal.species = "Perro";
        )",
        R"(saludar();)",
        R"(x = 4;)",
        R"(animal.species;)",
        R"(let perro = new Perro();)",
        R"(class Flor: Planta {})",
        R"(let flor: Planta;)",
    };
    checkErrors(test_strings, expect);
}

TEST_CASE("Check calls to undefined symbols", "[INVALID_KEYWORD_USE]") {
    std::string expect = "INVALID_KEYWORD_USE";

    std::vector<std::string> test_strings {
        "this;",
        "return;",
        "break;",
        "continue;",
    };
    checkErrors(test_strings, expect);
}

TEST_CASE("Unreachable code checks", "[UNREACHABLE_CODE]") {
    std::string expect = "UNREACHABLE_CODE";

    std::vector<std::string> test_strings {
        R"(
        function saludar1(): string {
            return "hola";
            let x = 0;
        })",
        R"(
        function saludar2() {
            for (let i = 0; i < 10; i = i + 1) {
                break;
                print("hola");
            }
        })",
        R"(
        function saludar3() {
            while (true) {
                continue;
                break;
            }
        })",
    };
    checkErrors(test_strings, expect);
}

TEST_CASE("Suffixes", "[INVALID_SUFFIX]") {
    std::string expect = "INVALID_SUFFIX";

    std::vector<std::string> test_strings {
        R"(
        class Animal {
            let nombre: string;

            function constructor(nombre: string) {
                this.nombre = nombre;
            }
        }

        let animal: Animal = new Animal("Firu");
 
        function saludar(): string {
            return "hola";
        }

        let array = [1, 2, 3];

        array();
        )",
        "array.len;",
        "animal[0];",
        "saludar[0];",
        "saludar.proc;",
        "animal();",
    };
    checkErrors(test_strings, expect);
}

TEST_CASE("Function calls", "[INCOMPLETE_CALL]") {
    std::string expect = "INCOMPLETE_CALL";

    std::vector<std::string> test_strings {
        R"(
        class Animal {
            let nombre: string;

            function constructor(nombre: string) {
                this.nombre = nombre;
            }
        }
 
        function saludar(mensaje: string) {
            print(mensaje);
        }

        let animal: Animal = new Animal();
        )",
        "saludar();",
        "saludar;",
    };
    checkErrors(test_strings, expect);
}

TEST_CASE("Constants", "[CONSTANT_MODIFICATION]") {
    std::string expect = "CONSTANT_MODIFICATION";

    std::vector<std::string> test_strings {
        R"(
        class Animal {
            const id = 0;
            let nombre: string;

            function constructor(nombre: string) {
                this.nombre = nombre;
            }
        }

        let animal1: Animal = new Animal("Firu");
        const animal2: Animal = new Animal("Toby");

        const year = 2025;

        year = 2026;
        )",
        "animal1.id = 1;",
        R"(animal2.nombre = "Lia";)",
    };
    checkErrors(test_strings, expect);
}

TEST_CASE("Function return", "[MISSING_RETURN]") {
    std::string expect = "MISSING_RETURN";

    std::vector<std::string> test_strings {
        R"(
        function saludar(mensaje: string): string {
            print(mensaje);
        })",
        R"(
        function saludar(mensaje: string): string {
            while (true) {
                return mensaje;
            }
            print(mensaje);
        })",
    };
    checkErrors(test_strings, expect);
}

TEST_CASE("Declarations", "[INVALID_DECLARATION]") {
    std::string expect = "INVALID_DECLARATION";

    std::vector<std::string> test_strings {
        R"(
        class Animal {
            let nombre: string;

            function constructor(nombre: string) {
                this.nombre = nombre;
            }
            function crear_perro() {
                class Perro {
                    let nombre: string;
                }
            }
        }
        )",
        "let x;",
    };
    checkErrors(test_strings, expect);
}
