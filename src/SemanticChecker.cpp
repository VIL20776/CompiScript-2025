#include <cstdio>
#include <string>
#include <print>
#include <regex>
#include <any>

#include "SymbolTable.h"
#include "SemanticChecker.h"

using namespace CompiScript;

// Helper functions

SymbolDataType getSymbolDataType(std::string type_name) {
    if (type_name == "integer") return SymbolDataType::INTEGER;
    if (type_name == "string") return SymbolDataType::STRING;
    if (type_name == "boolean") return SymbolDataType::BOOLEAN;
    return SymbolDataType::OBJECT;
}

std::string getSymbolDataTypeString(SymbolDataType type) {
    switch (type) {
        case SymbolDataType::UNDEFINED: return "undefined";
        case SymbolDataType::INTEGER: return "integer";
        case SymbolDataType::BOOLEAN: return "boolean";
        case SymbolDataType::STRING: return "string";
        case SymbolDataType::OBJECT: return "TODO";
        case SymbolDataType::NIL: return "null";
    }
    return "";
}

SemanticChecker::SemanticChecker(): table() {}
SemanticChecker::~SemanticChecker() {}

//SemanticChecker implementations

std::any SemanticChecker::visitProgram(CompiScriptParser::ProgramContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitStatement(CompiScriptParser::StatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitBlock(CompiScriptParser::BlockContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitVariableDeclaration(CompiScriptParser::VariableDeclarationContext *ctx) {

    auto name = ctx->Identifier()->getText();
    auto exists = table.lookup(name).second;
    if (exists) {
        std::printf("Variable '%s' was already declared.\n", name.c_str());
    }

    Symbol new_symbol = {.name = name, .type = SymbolType::VARIABLE };

    if (ctx->typeAnnotation() != nullptr) {
        auto symbol_type = std::any_cast<Symbol>(visitTypeAnnotation(ctx->typeAnnotation()));
        new_symbol.data_type = symbol_type.data_type;
        // new_symbol.type = symbol_type.type;
    }

    if (ctx->initializer() != nullptr) {
        auto initiallizer = std::any_cast<Symbol>(visitInitializer(ctx->initializer()));
        if (new_symbol.data_type != SymbolDataType::UNDEFINED && new_symbol.data_type != initiallizer.data_type) {
            std::printf("Variable '%s' not compatible with variable '%s'.\n", 
                         getSymbolDataTypeString(new_symbol.data_type).c_str(),
                         getSymbolDataTypeString(initiallizer.data_type).c_str());
        }

        new_symbol.value = initiallizer.value;
        new_symbol.data_type = initiallizer.data_type;
    }
    
    table.insert(new_symbol);

    return visitChildren(ctx);
}

std::any SemanticChecker::visitConstantDeclaration(CompiScriptParser::ConstantDeclarationContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitTypeAnnotation(CompiScriptParser::TypeAnnotationContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitInitializer(CompiScriptParser::InitializerContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitAssignment(CompiScriptParser::AssignmentContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitExpressionStatement(CompiScriptParser::ExpressionStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitPrintStatement(CompiScriptParser::PrintStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitIfStatement(CompiScriptParser::IfStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitWhileStatement(CompiScriptParser::WhileStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitDoWhileStatement(CompiScriptParser::DoWhileStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitForStatement(CompiScriptParser::ForStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitForeachStatement(CompiScriptParser::ForeachStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitBreakStatement(CompiScriptParser::BreakStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitContinueStatement(CompiScriptParser::ContinueStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitReturnStatement(CompiScriptParser::ReturnStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitTryCatchStatement(CompiScriptParser::TryCatchStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitSwitchStatement(CompiScriptParser::SwitchStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitSwitchCase(CompiScriptParser::SwitchCaseContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitDefaultCase(CompiScriptParser::DefaultCaseContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitFunctionDeclaration(CompiScriptParser::FunctionDeclarationContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitParameters(CompiScriptParser::ParametersContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitParameter(CompiScriptParser::ParameterContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitClassDeclaration(CompiScriptParser::ClassDeclarationContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitClassMember(CompiScriptParser::ClassMemberContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitExpression(CompiScriptParser::ExpressionContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitAssignExpr(CompiScriptParser::AssignExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitPropertyAssignExpr(CompiScriptParser::PropertyAssignExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitExprNoAssign(CompiScriptParser::ExprNoAssignContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitTernaryExpr(CompiScriptParser::TernaryExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitLogicalOrExpr(CompiScriptParser::LogicalOrExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitLogicalAndExpr(CompiScriptParser::LogicalAndExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitEqualityExpr(CompiScriptParser::EqualityExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitRelationalExpr(CompiScriptParser::RelationalExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitAdditiveExpr(CompiScriptParser::AdditiveExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitMultiplicativeExpr(CompiScriptParser::MultiplicativeExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitUnaryExpr(CompiScriptParser::UnaryExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitPrimaryExpr(CompiScriptParser::PrimaryExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitLiteralExpr(CompiScriptParser::LiteralExprContext *ctx) {
    Symbol new_symbol;
    if (ctx->arrayLiteral() != nullptr) {
        // TODO
    }

    new_symbol.value = ctx->getText();
    if (ctx->Literal() != nullptr) {
        auto literal = ctx->Literal()->getSymbol();
        if (std::regex_match(literal->getText(), std::regex("[0-9]+"))) 
            new_symbol.data_type = SymbolDataType::INTEGER;
         
        if (std::regex_match(literal->getText(), std::regex("\"([^\"\r\n])*\""))) 
            new_symbol.data_type = SymbolDataType::STRING;


    } else if (new_symbol.value == "true" || new_symbol.value == "false") {
        new_symbol.data_type = SymbolDataType::BOOLEAN;
    } else {
        new_symbol.data_type = SymbolDataType::NIL;
        new_symbol.value = "";
    }

    return std::make_any<Symbol>(new_symbol);
}

std::any SemanticChecker::visitLeftHandSide(CompiScriptParser::LeftHandSideContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitIdentifierExpr(CompiScriptParser::IdentifierExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitNewExpr(CompiScriptParser::NewExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitThisExpr(CompiScriptParser::ThisExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitCallExpr(CompiScriptParser::CallExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitIndexExpr(CompiScriptParser::IndexExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitPropertyAccessExpr(CompiScriptParser::PropertyAccessExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitArguments(CompiScriptParser::ArgumentsContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitArrayLiteral(CompiScriptParser::ArrayLiteralContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitType(CompiScriptParser::TypeContext *ctx) {
    auto symbol_type = std::any_cast<Symbol>(visitBaseType(ctx->baseType()));
    // TODO: Check if it is an array
    return std::make_any<Symbol>(symbol_type);
}

std::any SemanticChecker::visitBaseType(CompiScriptParser::BaseTypeContext *ctx) {
    Symbol symbol_type;
    symbol_type.data_type = getSymbolDataType(ctx->getText());
    return std::make_any<Symbol>(symbol_type);
}
