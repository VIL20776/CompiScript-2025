#include <fstream>
#include <string>
#include <print>

#include "CompiScriptParser.h"
#include "CompiScriptLexer.h"
#include "SemanticChecker.h"

int main (int argc, char** argv) {
    using namespace CompiScript;

    std::ifstream stream;
    if (argc < 2) {
        std::println(stderr, "Error: Not enough arguments given.");
        return 1;
    }
    
    std::string file_path(argv[1]);
    if (!file_path.ends_with(".cps")) {
        std::println(stderr, "Error: Invalid file given.");
        return 1;
    }
    stream.open(file_path.c_str());

    auto input = antlr4::ANTLRInputStream(stream);
    CompiScriptLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    CompiScriptParser parser(&tokens);
    SemanticChecker checker;
    checker.visitProgram(parser.program());

    for (int i = 2; i < argc; i++) {
        std::string option(argv[i]);
        if (option == "-print-tables") {
            auto table = checker.getSymbolTable();
            table.printTables();
        }
    }
    return 0;
}
