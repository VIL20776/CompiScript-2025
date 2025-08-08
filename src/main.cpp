#include "CompiScriptParser.h"
#include "CompiScriptLexer.h"
#include "SemanticChecker.h"

#include <fstream>


int main (int argc, char** argv) {
    using namespace CompiScript;

    std::ifstream stream;
    if (argc < 2) {
        return 1;
    }

    stream.open(argv[1]);

    auto input = antlr4::ANTLRInputStream(stream);
    CompiScriptLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    CompiScriptParser parser(&tokens);
    SemanticChecker checker;
    checker.visitProgram(parser.program());
    return 0;
}
