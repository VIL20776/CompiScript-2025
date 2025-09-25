#include <sstream>
#include <string>
#include <regex>
#include <any>

#include "SymbolTable.h"

#include "IRGenerator.h"

using namespace CompiScript;


IRGenerator::IRGenerator(SymbolTable* table): 
    table(table), 
    optimize(), 
    quadruplets(), 
    temp_count(0),
    class_def(false) {}

IRGenerator::~IRGenerator() {}

void IRGenerator::optimizeQuadruplets() {
    if (optimize.empty() && temp_count == 0) {
        quadruplets.append_range(optimize);
        return;
    }

    // TODO

    quadruplets.append_range(optimize);
    optimize.clear();
}

int IRGenerator::getSymbolSize(Symbol symbol) {
    switch (symbol.data_type) {
        case SymbolDataType::STRING:
        case SymbolDataType::INTEGER:
            return 4;
        case SymbolDataType::BOOLEAN:
        case SymbolDataType::NIL:
            return 1;
        case SymbolDataType::OBJECT: {
            auto class_symbol = table->lookup(symbol.parent).first;
            return class_symbol.size;
        }
        default:
            return 0;
    }
}

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
    table->enter();
    visitChildren(ctx);
    table->exit();

    return std::any();
}

std::any IRGenerator::visitVariableDeclaration(CompiScriptParser::VariableDeclarationContext *ctx) {
    auto dest = table->lookup(ctx->Identifier()->getText()).first;
    if (dest.value.empty()) {
        auto source = castSymbol(visitInitializer(ctx->initializer()));
        auto arg = (source.type == SymbolType::LITERAL) ? source.value : source.name;

        optimize.push_back({.arg1 = arg, .result = dest.label + dest.name});
        optimizeQuadruplets();
        temp_count = 0;
    } else {
        if (!dest.dimentions.empty()) {
            quadruplets.push_back({.op = "alloc", .arg1 = std::to_string(dest.size), .result = dest.label + dest.name});
            std::stringstream value_stream (dest.value);
            std::string value;
            int offset = 0;
            int type_size = getSymbolSize(dest); 

            while(std::getline(value_stream, value, ';')) {
                if (value.empty()) continue;

                quadruplets.push_back({.op = "+", .arg1 = dest.label + dest.name, .arg2 = std::to_string(offset), .result = "i"});
                quadruplets.push_back({.arg1 = value, .result = "i*"});
                offset += type_size;
            }
        } else {}
    }

    return std::any();
}

std::any IRGenerator::visitConstantDeclaration(CompiScriptParser::ConstantDeclarationContext *ctx) {
    auto target = table->lookup(ctx->Identifier()->getText()).first;
    auto source = castSymbol(visitExpression(ctx->expression()));
    auto arg = (source.type == SymbolType::LITERAL) ? source.value : source.name;

    optimize.push_back({.arg1 = arg, .result = target.name});
    optimizeQuadruplets();

    temp_count = 0;
    return std::any();
}

std::any IRGenerator::visitTypeAnnotation(CompiScriptParser::TypeAnnotationContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitInitializer(CompiScriptParser::InitializerContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitAssignment(CompiScriptParser::AssignmentContext *ctx) {
    if (ctx->expression().size() > 1) {
        //TODO
    }
    auto target = table->lookup(ctx->Identifier()->getText()).first;
    auto source = castSymbol(visitExpression(ctx->expression().at(0)));
    auto arg = (source.type == SymbolType::LITERAL) ? source.value : source.label + source.name;

    optimize.push_back({.arg1 = arg, .result = target.label + target.name});
    optimizeQuadruplets();

    temp_count = 0;
    return visitChildren(ctx);
}

std::any IRGenerator::visitExpressionStatement(CompiScriptParser::ExpressionStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitPrintStatement(CompiScriptParser::PrintStatementContext *ctx) {
    auto symbol = castSymbol(visitExpression(ctx->expression()));
    if (symbol.data_type != SymbolDataType::STRING) {
        auto arg = (symbol.type == SymbolType::LITERAL) ? symbol.value : symbol.label + symbol.name;
        optimize.push_back({.op = "to_str", .arg1 = arg, .arg2 = std::to_string(getSymbolSize(symbol)), .result = "p"});
    }
    optimize.push_back({.op = "print"});
    optimizeQuadruplets();

    return std::any();
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
    auto ret = castSymbol(visitExpression(ctx->expression()));
    auto arg = (ret.type == SymbolType::LITERAL) ? ret.value : ret.label + ret.name;
    optimize.push_back({.op = "return", .arg1 = arg});
    optimizeQuadruplets();
    return std::any();
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
    auto function = table->lookup(ctx->Identifier()->getText()).first;

    quadruplets.push_back({.op = "begin", .arg1 = function.label + function.name});
    if (class_def) 
        quadruplets.push_back({.op = "param", .result = function.label + "this"});
    
    for (auto arg: function.arg_list) 
        quadruplets.push_back({.op = "param", .result = arg.label + arg.name});

    if (class_def && function.name == "constructor") {
        auto self = table->lookup("this").first;
        quadruplets.push_back({.op = "alloc", .arg1 = std::to_string(self.size), .result = "this"});
    }
    
    visitBlock(ctx->block());

    quadruplets.push_back({.op = "end", .arg1 = function.label + function.name});
    return std::any();
}

std::any IRGenerator::visitParameters(CompiScriptParser::ParametersContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitParameter(CompiScriptParser::ParameterContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitClassDeclaration(CompiScriptParser::ClassDeclarationContext *ctx) {
    class_def = true;
    table->enter();
    for (auto member: ctx->classMember())
        visitClassMember(member);
    table->exit();
    class_def = false;
    
    return std::any();
}

std::any IRGenerator::visitClassMember(CompiScriptParser::ClassMemberContext *ctx) {
    if (ctx->functionDeclaration())
        visitFunctionDeclaration(ctx->functionDeclaration());

    return std::any();
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

            auto arg1 = (first_symbol.type == SymbolType::LITERAL) ? first_symbol.value : first_symbol.label + first_symbol.name;
            auto arg2 = (second_symbol.type == SymbolType::LITERAL) ? second_symbol.value : second_symbol.label + second_symbol.name;
            auto temp = "t" + std::to_string(temp_count++);

            optimize.push_back({.op = "||", .arg1 = arg1, .arg2 = arg2, .result = temp});

            first_symbol.name = temp;
            first_symbol.label = "";
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

            auto arg1 = (first_symbol.type == SymbolType::LITERAL) ? first_symbol.value : first_symbol.label + first_symbol.name;
            auto arg2 = (second_symbol.type == SymbolType::LITERAL) ? second_symbol.value : second_symbol.label + second_symbol.name;
            auto temp = "t" + std::to_string(temp_count++);

            optimize.push_back({.op = "&&", .arg1 = arg1, .arg2 = arg2, .result = temp});

            first_symbol.name = temp;
            first_symbol.label = "";
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

            auto arg1 = (first_symbol.type == SymbolType::LITERAL) ? first_symbol.value : first_symbol.label + first_symbol.name;
            auto arg2 = (second_symbol.type == SymbolType::LITERAL) ? second_symbol.value : second_symbol.label + second_symbol.name;

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

            optimize.push_back({.op = op, .arg1 = arg1, .arg2 = arg2, .result = temp});

            first_symbol.name = temp;
            first_symbol.label = "";
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

            auto arg1 = (first_symbol.type == SymbolType::LITERAL) ? first_symbol.value : first_symbol.label + first_symbol.name;
            auto arg2 = (second_symbol.type == SymbolType::LITERAL) ? second_symbol.value : second_symbol.label + second_symbol.name;

            auto temp = "t" + std::to_string(temp_count++);

            auto op = ctx->children.at(op_index)->getText();
            optimize.push_back({.op = op, .arg1 = arg1, .arg2 = arg2, .result = temp});
            op_index += 2;

            first_symbol.name = temp;
            first_symbol.label = "";
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

            auto arg1 = (first_symbol.type == SymbolType::LITERAL) ? first_symbol.value : first_symbol.label + first_symbol.name;
            auto arg2 = (second_symbol.type == SymbolType::LITERAL) ? second_symbol.value : second_symbol.label + second_symbol.name;

            auto temp = "t" + std::to_string(temp_count++);

            if (first_symbol.data_type == SymbolDataType::STRING) {
                if (second_symbol.data_type != SymbolDataType::STRING) {
                    optimize.push_back({.op = "to_str", .arg1 = arg2, .arg2 = std::to_string(getSymbolSize(second_symbol)), .result = temp});
                    arg2 = temp;
                    temp = "t" + std::to_string(temp_count++);
                }

                optimize.push_back({.op = "concat", .arg1 = arg1, .arg2 = arg2, .result = temp});
            } else {
                auto op = ctx->children.at(op_index)->getText();
                optimize.push_back({.op = op, .arg1 = arg1, .arg2 = arg2, .result = temp});
                op_index += 2;
            }

            first_symbol.name = temp;
            first_symbol.label = "";
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

            auto arg1 = (first_symbol.type == SymbolType::LITERAL) ? first_symbol.value : first_symbol.label + first_symbol.name;
            auto arg2 = (second_symbol.type == SymbolType::LITERAL) ? second_symbol.value : second_symbol.label + second_symbol.name;

            auto temp = "t" + std::to_string(temp_count++);

            auto op = ctx->children.at(op_index)->getText();
            optimize.push_back({.op = op, .arg1 = arg1, .arg2 = arg2, .result = temp});
            op_index += 2;

            first_symbol.name = temp;
            first_symbol.label = "";
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

        auto arg = (symbol.type == SymbolType::LITERAL) ? symbol.value : symbol.label + symbol.name;
        auto temp = "t" + std::to_string(temp_count++);

        optimize.push_back({.op = op, .arg1 = arg, .result = temp});

        symbol.name = temp;
        symbol.label = "";
        symbol.type = SymbolType::VARIABLE;
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
    if (ctx->Literal() != nullptr) {
        auto literal = ctx->Literal()->getSymbol();
        if (std::regex_match(literal->getText(), std::regex("[0-9]+"))) {
            new_symbol.data_type = SymbolDataType::INTEGER;
            new_symbol.size = 4;
        }

        if (std::regex_match(literal->getText(), std::regex("\"([^\"\r\n])*\""))) {
            new_symbol.data_type = SymbolDataType::STRING;
            new_symbol.size = 4;
        }


    } else if (new_symbol.value == "true" || new_symbol.value == "false") {
        new_symbol.data_type = SymbolDataType::BOOLEAN;
        new_symbol.size = 1;
    } else {
        new_symbol.data_type = SymbolDataType::NIL;
        new_symbol.value = "null";
        new_symbol.size = 1;
    }
    return makeAny(new_symbol);
}

std::any IRGenerator::visitLeftHandSide(CompiScriptParser::LeftHandSideContext *ctx) {
    auto atom = castSymbol(visit(ctx->primaryAtom()));
    for (auto suffixOp: ctx->suffixOp()) {
        auto suffix = castSymbol(visit(suffixOp));
        if (atom.type == SymbolType::FUNCTION && suffix.type == SymbolType::ARGUMENT) {
            for (auto it = suffix.arg_list.rbegin(); it != suffix.arg_list.rend(); it++) {
                auto arg = (it->type == SymbolType::LITERAL) ? it->value : it->label + it->name;
                optimize.push_back({.op = "push", .arg1 = arg});
            }
             
            if (suffix.data_type == SymbolDataType::NIL)
                optimize.push_back({.op = "call", .arg1 = atom.label + atom.name});
            else
                optimize.push_back({.op = "call", .arg1 = atom.label + atom.name, .result = "ret"});

            atom.name = "ret";
            atom.label = "";
            atom.type = SymbolType::VARIABLE;
        }
        else
        if (!atom.parent.empty() && suffix.type == SymbolType::PROPERTY) {
            
        }
        else
        if (!atom.dimentions.empty() && suffix.data_type == SymbolDataType::INTEGER) {
            auto arg = (suffix.type == SymbolType::LITERAL) ? suffix.value : suffix.label + suffix.name;
            // auto temp = "t" + std::to_string(temp_count++);
            optimize.push_back({.arg1 = arg, .result = "t0"});
            for (int i = 1; i < atom.dimentions.size(); i++) {
                optimize.push_back({.op = "*", .arg1 = "t0", .arg2 = std::to_string(atom.dimentions.at(i)), .result = "t0"});
            }
            optimize.push_back({.op = "*", .arg1 = "t0", .arg2 = std::to_string(getSymbolSize(atom)), .result = "t0"});
            optimize.push_back({.op = "+", .arg1 = atom.label + atom.name, .arg2 = "t0", .result = "i"});

            if (atom.dimentions.size() - 1 > 0) {
                atom.name = "i";
            } else {
                optimize.push_back({.arg1 = "i*", .result = "t0"});
                atom.name = "t0";
            }

            atom.label = "";
            atom.dimentions = std::vector(atom.dimentions.begin() + 1, atom.dimentions.end());
        }
    }
    return makeAny(atom);
}

std::any IRGenerator::visitIdentifierExpr(CompiScriptParser::IdentifierExprContext *ctx) {
    return table->lookup(ctx->Identifier()->getText()).first;
}

std::any IRGenerator::visitNewExpr(CompiScriptParser::NewExprContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitThisExpr(CompiScriptParser::ThisExprContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitCallExpr(CompiScriptParser::CallExprContext *ctx) {
    if (ctx->arguments() == nullptr)
        return Symbol({.type = SymbolType::ARGUMENT});

    return visitArguments(ctx->arguments());
}

std::any IRGenerator::visitIndexExpr(CompiScriptParser::IndexExprContext *ctx) {
    Symbol array_index = castSymbol(visitExpression(ctx->expression()));
    return array_index;
}

std::any IRGenerator::visitPropertyAccessExpr(CompiScriptParser::PropertyAccessExprContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitArguments(CompiScriptParser::ArgumentsContext *ctx) {
    Symbol symbol_arguments = {.type = SymbolType::ARGUMENT};
    for (auto expr: ctx->expression()) {
        symbol_arguments.arg_list.push_back(castSymbol(visitExpression(expr)));
    }
    return makeAny(symbol_arguments);
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

