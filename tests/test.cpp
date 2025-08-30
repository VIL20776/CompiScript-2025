#include "CompiScriptLexer.h"
#include "CompiScriptParser.h"
#include "SemanticChecker.h"

#include "test.h"

void test_stream(const std::string &stream, CompiScript::SemanticChecker *checker) {
    auto input = antlr4::ANTLRInputStream(stream);
    CompiScript::CompiScriptLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    CompiScript::CompiScriptParser parser(&tokens);
    checker->visitProgram(parser.program());
}
