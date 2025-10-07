#include <fstream>
#include <string>
#include <print>

#include "CompiScriptParser.h"
#include "CompiScriptLexer.h"
#include "SemanticChecker.h"
#include "IRGenerator.h"

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

    auto table = checker.getSymbolTable();
    auto ir = CompiScript::IRGenerator(&table);

    auto input2 = antlr4::ANTLRInputStream(stream);
    CompiScript::CompiScriptLexer lexer2(&input2);
    antlr4::CommonTokenStream tokens2(&lexer2);
    CompiScript::CompiScriptParser parser2(&tokens2);
    ir.visitProgram(parser2.program());

    for (int i = 2; i < argc; i++) {
        std::string option(argv[i]);
        if (option == "-print-tables") {
            auto table = checker.getSymbolTable();
            table.printTables();
        }
        if (option == "-tac") {

            std::println("{}", ir.getTAC().c_str());

            std::ofstream file("tac.ir");
            file << ir.getTAC();
        }
    }

    stream.close();
    return 0;
}
