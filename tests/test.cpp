#include "CompiScriptLexer.h"
#include "CompiScriptParser.h"
#include "SemanticChecker.h"
#include "IRGenerator.h"
#include "Mips.h"

#include "test.h"

void test_stream(const std::string &stream, CompiScript::SemanticChecker *checker) {
    auto input = antlr4::ANTLRInputStream(stream);
    CompiScript::CompiScriptLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    CompiScript::CompiScriptParser parser(&tokens);
    checker->visitProgram(parser.program());
}

std::string test_ir_gen(const std::string &stream) {
    CompiScript::SemanticChecker checker;
    auto input = antlr4::ANTLRInputStream(stream);
    CompiScript::CompiScriptLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    CompiScript::CompiScriptParser parser(&tokens);
    checker.visitProgram(parser.program());

    auto table = checker.getSymbolTable();
    auto ir = CompiScript::IRGenerator(&table);
    
    auto input2 = antlr4::ANTLRInputStream(stream);
    CompiScript::CompiScriptLexer lexer2(&input2);
    antlr4::CommonTokenStream tokens2(&lexer2);
    CompiScript::CompiScriptParser parser2(&tokens2);
    ir.visitProgram(parser2.program());
    return  ir.getTAC();
}

std::string test_mips_gen(const std::string &stream) {
    CompiScript::SemanticChecker checker;
    auto input = antlr4::ANTLRInputStream(stream);
    CompiScript::CompiScriptLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    CompiScript::CompiScriptParser parser(&tokens);
    checker.visitProgram(parser.program());

    auto table = checker.getSymbolTable();
    auto ir = CompiScript::IRGenerator(&table);
    
    auto input2 = antlr4::ANTLRInputStream(stream);
    CompiScript::CompiScriptLexer lexer2(&input2);
    antlr4::CommonTokenStream tokens2(&lexer2);
    CompiScript::CompiScriptParser parser2(&tokens2);
    ir.visitProgram(parser2.program());

    CompiScript::Mips asm_gen(ir.getQuadruplets());

    return  asm_gen.generateAssembly();
}
