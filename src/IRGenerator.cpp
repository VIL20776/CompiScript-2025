#include <string>
#include <print>
#include <regex>
#include <any>

#include "SymbolTable.h"

#include "IRGenerator.h"

using namespace CompiScript;

IRGenerator::IRGenerator(SymbolTable* table): table(table), temp_count(), quadruplets() {}

IRGenerator::~IRGenerator() {};

std::string IRGenerator::getTAC() {
    std::string tac;
    for (auto quad: quadruplets) {
        auto temp = quad.op + " " + quad.arg1 + " " + quad.arg2 + "\n";

        if (!quad.result.empty()) 
            temp = quad.result + " = " + temp;
        tac = tac + temp;
    }
    return tac;
}

std::any IRGenerator::visitProgram(CompiScriptParser::ProgramContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitStatement(CompiScriptParser::StatementContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitBlock(CompiScriptParser::BlockContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitVariableDeclaration(CompiScriptParser::VariableDeclarationContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitConstantDeclaration(CompiScriptParser::ConstantDeclarationContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitTypeAnnotation(CompiScriptParser::TypeAnnotationContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitInitializer(CompiScriptParser::InitializerContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitAssignment(CompiScriptParser::AssignmentContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitExpressionStatement(CompiScriptParser::ExpressionStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitPrintStatement(CompiScriptParser::PrintStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitIfStatement(CompiScriptParser::IfStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitWhileStatement(CompiScriptParser::WhileStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitDoWhileStatement(CompiScriptParser::DoWhileStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitForStatement(CompiScriptParser::ForStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitForeachStatement(CompiScriptParser::ForeachStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitBreakStatement(CompiScriptParser::BreakStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitContinueStatement(CompiScriptParser::ContinueStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitReturnStatement(CompiScriptParser::ReturnStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitTryCatchStatement(CompiScriptParser::TryCatchStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitSwitchStatement(CompiScriptParser::SwitchStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitSwitchCase(CompiScriptParser::SwitchCaseContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitDefaultCase(CompiScriptParser::DefaultCaseContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitFunctionDeclaration(CompiScriptParser::FunctionDeclarationContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitParameters(CompiScriptParser::ParametersContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitParameter(CompiScriptParser::ParameterContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitClassDeclaration(CompiScriptParser::ClassDeclarationContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitClassMember(CompiScriptParser::ClassMemberContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitExpression(CompiScriptParser::ExpressionContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitAssignExpr(CompiScriptParser::AssignExprContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitPropertyAssignExpr(CompiScriptParser::PropertyAssignExprContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitExprNoAssign(CompiScriptParser::ExprNoAssignContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitTernaryExpr(CompiScriptParser::TernaryExprContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitLogicalOrExpr(CompiScriptParser::LogicalOrExprContext *ctx) {
    if (ctx->logicalAndExpr().size() > 1) {
        auto first_symbol = castSymbol(visitLogicalAndExpr(ctx->logicalAndExpr().at(0)));
        for (int i = 1; i < ctx->logicalAndExpr().size(); i++) {
            auto second_symbol = castSymbol(visitLogicalAndExpr(ctx->logicalAndExpr().at(i)));

            auto arg1 = (first_symbol.type == SymbolType::LITERAL) ? first_symbol.value : first_symbol.name;
            auto arg2 = (second_symbol.type == SymbolType::LITERAL) ? second_symbol.value : second_symbol.name;
            auto temp = "t" + std::to_string(temp_count++);

            quadruplets.push_back({.op = "||", .arg1 = arg1, .arg2 = arg2, .result = temp});

            first_symbol.name = temp;
            first_symbol.type = SymbolType::VARIABLE;
        }
        return makeAny(first_symbol);
    }
    return visitChildren(ctx);
}

std::any IRGenerator::visitLogicalAndExpr(CompiScriptParser::LogicalAndExprContext *ctx) {
    if (ctx->equalityExpr().size() > 1) {
        auto first_symbol = castSymbol(visitEqualityExpr(ctx->equalityExpr().at(0)));
        for (int i = 1; i < ctx->equalityExpr().size(); i++) {
            auto second_symbol = castSymbol(visitEqualityExpr(ctx->equalityExpr().at(i)));

            auto arg1 = (first_symbol.type == SymbolType::LITERAL) ? first_symbol.value : first_symbol.name;
            auto arg2 = (second_symbol.type == SymbolType::LITERAL) ? second_symbol.value : second_symbol.name;
            auto temp = "t" + std::to_string(temp_count++);

            quadruplets.push_back({.op = "&&", .arg1 = arg1, .arg2 = arg2, .result = temp});

            first_symbol.name = temp;
            first_symbol.type = SymbolType::VARIABLE;
        }
        return makeAny(first_symbol);
    }
    return visitChildren(ctx);
}

std::any IRGenerator::visitEqualityExpr(CompiScriptParser::EqualityExprContext *ctx) {
    if (ctx->relationalExpr().size() > 1) {
        int op_index = 1;
        auto first_symbol = castSymbol(visitRelationalExpr(ctx->relationalExpr().at(0)));
        for (int i = 1; i < ctx->relationalExpr().size(); i++) {
            auto second_symbol = castSymbol(visitRelationalExpr(ctx->relationalExpr().at(i)));

            auto arg1 = (first_symbol.type == SymbolType::LITERAL) ? first_symbol.value : first_symbol.name;
            auto arg2 = (second_symbol.type == SymbolType::LITERAL) ? second_symbol.value : second_symbol.name;
            auto temp = "t" + std::to_string(temp_count++);

            auto op = ctx->children.at(op_index)->getText();
            op_index += 2;
            if (first_symbol.data_type == SymbolDataType::STRING) {
                if (op == "==")
                    quadruplets.push_back({.op = "streql", .arg1 = arg1, .arg2 = arg2, .result = temp});

                if (op == "!=")
                    quadruplets.push_back({.op = "strneq", .arg1 = arg1, .arg2 = arg2, .result = temp});

                continue;
            } 

            quadruplets.push_back({.op = op, .arg1 = arg1, .arg2 = arg2, .result = temp});

            first_symbol.name = temp;
            first_symbol.type = SymbolType::VARIABLE;
        }
        return makeAny(first_symbol);
    }
    return visitChildren(ctx);
}

std::any IRGenerator::visitRelationalExpr(CompiScriptParser::RelationalExprContext *ctx) {
    if (ctx->additiveExpr().size() > 1) {
        int op_index = 1;
        auto first_symbol = castSymbol(visitAdditiveExpr(ctx->additiveExpr().at(0)));
        for (int i = 1; i < ctx->additiveExpr().size(); i++) {
            auto second_symbol = castSymbol(visitAdditiveExpr(ctx->additiveExpr().at(i)));

            auto arg1 = (first_symbol.type == SymbolType::LITERAL) ? first_symbol.value : first_symbol.name;
            auto arg2 = (second_symbol.type == SymbolType::LITERAL) ? second_symbol.value : second_symbol.name;
            auto temp = "t" + std::to_string(temp_count++);

            auto op = ctx->children.at(op_index)->getText();
            quadruplets.push_back({.op = op, .arg1 = arg1, .arg2 = arg2, .result = temp});
            op_index += 2;

            first_symbol.name = temp;
            first_symbol.type = SymbolType::VARIABLE;
        }
        return makeAny(first_symbol);
    }
    return visitChildren(ctx);
}

std::any IRGenerator::visitAdditiveExpr(CompiScriptParser::AdditiveExprContext *ctx) {
    if (ctx->multiplicativeExpr().size() > 1) {
        int op_index = 1;
        auto first_symbol = castSymbol(visitMultiplicativeExpr(ctx->multiplicativeExpr().at(0)));
        for (int i = 1; i < ctx->multiplicativeExpr().size(); i++) {
            auto second_symbol = castSymbol(visitMultiplicativeExpr(ctx->multiplicativeExpr().at(i)));

            auto arg1 = (first_symbol.type == SymbolType::LITERAL) ? first_symbol.value : first_symbol.name;
            auto arg2 = (second_symbol.type == SymbolType::LITERAL) ? second_symbol.value : second_symbol.name;
            auto temp = "t" + std::to_string(temp_count++);

            if (first_symbol.data_type == SymbolDataType::STRING) {
                if (second_symbol.data_type != SymbolDataType::STRING) {
                    arg2 = temp;
                    quadruplets.push_back({.op = "to_str", .arg1 = arg1, .result = temp});
                    temp = "t" + std::to_string(temp_count++);
                }

                quadruplets.push_back({.op = "concat", .arg1 = arg1, .arg2 = arg2, .result = temp});
            } else {
                auto op = ctx->children.at(op_index)->getText();
                quadruplets.push_back({.op = op, .arg1 = arg1, .arg2 = arg2, .result = temp});
                op_index += 2;
            }

            first_symbol.name = temp;
            first_symbol.type = SymbolType::VARIABLE;
        }
        return makeAny(first_symbol);
    }
    return visitChildren(ctx);
}

std::any IRGenerator::visitMultiplicativeExpr(CompiScriptParser::MultiplicativeExprContext *ctx) {
    if (ctx->unaryExpr().size() > 1) {
        int op_index = 1;
        auto first_symbol = castSymbol(visitUnaryExpr(ctx->unaryExpr().at(0)));
        for (int i = 1; i < ctx->unaryExpr().size(); i++) {
            auto second_symbol = castSymbol(visitUnaryExpr(ctx->unaryExpr().at(i)));

            auto arg1 = (first_symbol.type == SymbolType::LITERAL) ? first_symbol.value : first_symbol.name;
            auto arg2 = (second_symbol.type == SymbolType::LITERAL) ? second_symbol.value : second_symbol.name;
            auto temp = "t" + std::to_string(temp_count++);

            auto op = ctx->children.at(op_index)->getText();
            quadruplets.push_back({.op = op, .arg1 = arg1, .arg2 = arg2, .result = temp});
            op_index += 2;

            first_symbol.name = temp;
            first_symbol.type = SymbolType::VARIABLE;
        }
        return makeAny(first_symbol);
    }
    return visitChildren(ctx);
}

std::any IRGenerator::visitUnaryExpr(CompiScriptParser::UnaryExprContext *ctx) {
    if (ctx->unaryExpr() != nullptr) {
        auto unary = ctx->unaryExpr();
        auto op = ctx->getStart()->getText();
        auto symbol = castSymbol(visitUnaryExpr(unary));

        auto arg = (symbol.type == SymbolType::LITERAL) ? symbol.value : symbol.name;
        auto temp = "t" + std::to_string(temp_count++);

        quadruplets.push_back({.op = op, .arg1 = arg, .result = temp});

        symbol.name = temp;
        return makeAny(symbol);
    }

    return visitChildren(ctx);
}

std::any IRGenerator::visitPrimaryExpr(CompiScriptParser::PrimaryExprContext *ctx) {
    if (ctx->expression() != nullptr) return visitExpression(ctx->expression());
    if (ctx->leftHandSide() != nullptr) return visitLeftHandSide(ctx->leftHandSide());
    return visitLiteralExpr(ctx->literalExpr());
}

std::any IRGenerator::visitLiteralExpr(CompiScriptParser::LiteralExprContext *ctx) {
    if (ctx->arrayLiteral() != nullptr)
        return makeAny(castSymbol(visitArrayLiteral(ctx->arrayLiteral())));

    Symbol new_symbol;
    new_symbol.value = ctx->getText();
    new_symbol.type = SymbolType::LITERAL;
    new_symbol.size = 32; // TODO: Assign different sizes
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
        new_symbol.value = "null";
    }


    return makeAny(new_symbol);
}

std::any IRGenerator::visitLeftHandSide(CompiScriptParser::LeftHandSideContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitIdentifierExpr(CompiScriptParser::IdentifierExprContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitNewExpr(CompiScriptParser::NewExprContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitThisExpr(CompiScriptParser::ThisExprContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitCallExpr(CompiScriptParser::CallExprContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitIndexExpr(CompiScriptParser::IndexExprContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitPropertyAccessExpr(CompiScriptParser::PropertyAccessExprContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitArguments(CompiScriptParser::ArgumentsContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitArrayLiteral(CompiScriptParser::ArrayLiteralContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitType(CompiScriptParser::TypeContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitBaseType(CompiScriptParser::BaseTypeContext *ctx) {
    return visitChildren(ctx);
}

