#include <sstream>
#include <print>
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
    begin_label(),
    end_label(),
    temp_count(0),
    label_count(0),
    func_def(false),
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

const std::vector<Quad>& IRGenerator::getQuadruplets() {
    return quadruplets;
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
    // std::println("{}", ctx->getText().c_str());
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
        auto arg = source.label + source.name;

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
        } else {
            quadruplets.push_back({.arg1 = dest.value, .result = dest.label + dest.name});
        }
    }

    if (func_def)
        registry.push_back(dest.label + dest.name);

    return std::any();
}

std::any IRGenerator::visitConstantDeclaration(CompiScriptParser::ConstantDeclarationContext *ctx) {
    auto dest = table->lookup(ctx->Identifier()->getText()).first;
    if (dest.value.empty()) {
        auto source = castSymbol(visitExpression(ctx->expression()));
        auto arg = source.label + source.name;

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
        } else {
            quadruplets.push_back({.arg1 = dest.value, .result = dest.label + dest.name});
        }
    }

    if (func_def)
        registry.push_back(dest.label + dest.name);

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
        auto target = castSymbol(visitExpression(ctx->expression().at(0)));
        auto prop = table->get_property(target.parent, ctx->Identifier()->getText()).first;
        auto source = castSymbol(visitExpression(ctx->expression().at(1)));
        auto arg = (source.type == SymbolType::LITERAL) ? source.value : source.label + source.name;

        optimize.push_back({.op = "+", .arg1 = target.label + target.name, .arg2 = std::to_string(prop.offset), .result = "i"});
        optimize.push_back({.arg1 = arg, .result = "i*"});
    } else {
        auto target = table->lookup(ctx->Identifier()->getText(), false).first;
        auto source = castSymbol(visitExpression(ctx->expression().at(0)));
        auto arg = (source.type == SymbolType::LITERAL) ? source.value : source.label + source.name;

        optimize.push_back({.arg1 = arg, .result = target.label + target.name});
    }
    optimizeQuadruplets();

    temp_count = 0;
    return std::any();
}

std::any IRGenerator::visitExpressionStatement(CompiScriptParser::ExpressionStatementContext *ctx) {
    visitChildren(ctx);
    optimizeQuadruplets();
    temp_count = 0;
    return std::any();
}

std::any IRGenerator::visitPrintStatement(CompiScriptParser::PrintStatementContext *ctx) {
    auto symbol = castSymbol(visitExpression(ctx->expression()));
    auto arg = (symbol.type == SymbolType::LITERAL) ? symbol.value : symbol.label + symbol.name;
    if (symbol.data_type != SymbolDataType::STRING)
        optimize.push_back({.op = "to_str", .arg1 = arg, .arg2 = std::to_string(getSymbolSize(symbol)), .result = "p"});
    else 
        optimize.push_back({.arg1 = arg, .result = "p"});
    
    optimize.push_back({.op = "print"});
    optimizeQuadruplets();
    temp_count = 0;

    return std::any();
}

std::any IRGenerator::visitIfStatement(CompiScriptParser::IfStatementContext *ctx) {
    auto expr = castSymbol(visitExpression(ctx->expression()));
    auto arg = (expr.type == SymbolType::LITERAL) ? expr.value : expr.label + expr.name;
    auto label = "l" + std::to_string(label_count++);
    auto else_label = "l" + std::to_string(label_count++);
    
    optimize.push_back({.op = "if", .arg1 = arg, .arg2 = label});
    optimize.push_back({.op = "goto", .arg1 = else_label});
    optimizeQuadruplets();
    temp_count = 0;

    quadruplets.push_back({.op = "tag", .arg1 = label});
    visitBlock(ctx->block().at(0));
    quadruplets.push_back({.op = "tag", .arg1 = else_label});

    if (ctx->block().size() == 2)
        visitBlock(ctx->block().at(1));

    return std::any();
}

std::any IRGenerator::visitWhileStatement(CompiScriptParser::WhileStatementContext *ctx) {
    auto prev_begin = begin_label;
    auto prev_end = end_label;

    begin_label = "l" + std::to_string(label_count++);
    end_label = "l" + std::to_string(label_count++);

    optimize.push_back({.op = "tag", .arg1 = begin_label});

    auto expr = castSymbol(visitExpression(ctx->expression()));
    auto arg = (expr.type == SymbolType::LITERAL) ? expr.value : expr.label + expr.name;
    
    optimize.push_back({.op = "ifnot", .arg1 = arg, .arg2 = end_label});
    optimizeQuadruplets();
    temp_count = 0;

    visitBlock(ctx->block());

    quadruplets.push_back({.op = "goto", .arg1 = begin_label});
    quadruplets.push_back({.op = "tag", .arg1 = end_label});

    begin_label = prev_begin;
    end_label = prev_end;
    return std::any();
}

std::any IRGenerator::visitDoWhileStatement(CompiScriptParser::DoWhileStatementContext *ctx) {
    auto prev_begin = begin_label;
    auto prev_end = end_label;

    begin_label = "l" + std::to_string(label_count++);
    end_label = "l" + std::to_string(label_count++);

    quadruplets.push_back({.op = "tag", .arg1 = begin_label});

    visitBlock(ctx->block());
 
    auto expr = castSymbol(visitExpression(ctx->expression()));
    auto arg = (expr.type == SymbolType::LITERAL) ? expr.value : expr.label + expr.name;   

    optimize.push_back({.op = "if", .arg1 = arg, .arg2 = begin_label});
    optimizeQuadruplets();
    temp_count = 0;

    quadruplets.push_back({.op = "tag", .arg1 = end_label}); 

    begin_label = prev_begin;
    end_label = prev_end;
    return std::any();
}

std::any IRGenerator::visitForStatement(CompiScriptParser::ForStatementContext *ctx) {
    auto prev_begin = begin_label;
    auto prev_end = end_label;

    begin_label = "l" + std::to_string(label_count++);
    end_label = "l" + std::to_string(label_count++);

    if (ctx->variableDeclaration() != nullptr)
        visitVariableDeclaration(ctx->variableDeclaration());
    if (ctx->assignment() != nullptr)
        visitAssignment(ctx->assignment());

    quadruplets.push_back({.op = "tag", .arg1 = begin_label});
    if (ctx->expression().size() > 0) {
        auto expr = castSymbol(visitExpression(ctx->expression().at(0)));
        auto arg = (expr.type == SymbolType::LITERAL) ? expr.value : expr.label + expr.name;
        optimize.push_back({.op = "ifnot", .arg1 = arg, .arg2 = end_label});
        optimizeQuadruplets();
        temp_count = 0;
    }

    visitBlock(ctx->block());

    if (ctx->expression().size() > 1) {
        visitExpression(ctx->expression().at(1));
        optimizeQuadruplets();
        temp_count = 0;
    }
    quadruplets.push_back({.op = "goto", .arg1 = begin_label});
    quadruplets.push_back({.op = "tag", .arg1 = end_label});

    begin_label = prev_begin;
    end_label = prev_end;
    return std::any();
}

std::any IRGenerator::visitForeachStatement(CompiScriptParser::ForeachStatementContext *ctx) {
    auto prev_begin = begin_label;
    auto prev_end = end_label;

    begin_label = "l" + std::to_string(label_count++);
    end_label = "l" + std::to_string(label_count++);    

    auto expr = castSymbol(visitExpression(ctx->expression()));
    auto arg = expr.label + expr.name;
    auto target = table->lookup(ctx->Identifier()->getText()).first;

    optimizeQuadruplets();
    temp_count = 0;

    quadruplets.push_back({.arg1 = arg, .result = "i"});
    quadruplets.push_back({.op = "tag", .arg1 = begin_label});
    if (target.dimentions.empty())
        quadruplets.push_back({.arg1 = "i*", .result = target.label + target.name});
    else
        quadruplets.push_back({.arg1 = "i", .result = target.label + target.name});

    int limit = expr.size;
    int offset = getSymbolSize(expr);
    for (auto i = 1; i < expr.dimentions.size(); i++)
        offset *= expr.dimentions.at(i);


    visitBlock(ctx->block());

    quadruplets.push_back({.op = "+", .arg1 = arg, .arg2 = std::to_string(offset), .result = "i"});
    quadruplets.push_back({.op = "+", .arg1 = arg, .arg2 = std::to_string(limit), .result = "t0"});
    quadruplets.push_back({.op = "<", .arg1 = "i", .arg2 = "t0", .result = "t0"});
    quadruplets.push_back({.op = "if", .arg1 = "t0", .arg2 = begin_label});

    quadruplets.push_back({.op = "tag", .arg1 = end_label});

    begin_label = prev_begin;
    end_label = prev_end;
    return std::any();
}

std::any IRGenerator::visitBreakStatement(CompiScriptParser::BreakStatementContext *ctx) {
    quadruplets.push_back({.op = "goto", .arg1 = end_label});
    return std::any();
}

std::any IRGenerator::visitContinueStatement(CompiScriptParser::ContinueStatementContext *ctx) {
    quadruplets.push_back({.op = "goto", .arg1 = begin_label});
    return std::any();
}

std::any IRGenerator::visitReturnStatement(CompiScriptParser::ReturnStatementContext *ctx) {
    auto ret = castSymbol(visitExpression(ctx->expression()));
    auto arg = (ret.type == SymbolType::LITERAL) ? ret.value : ret.label + ret.name;
    optimize.push_back({.op = "return", .arg1 = arg});
    optimizeQuadruplets();
    temp_count = 0;
    return std::any();
}

std::any IRGenerator::visitTryCatchStatement(CompiScriptParser::TryCatchStatementContext *ctx) {
    auto catch_label = "l" + std::to_string(label_count++);

    quadruplets.push_back({.arg1 = catch_label, .result = "catch"});
    visitBlock(ctx->block().at(0));
    quadruplets.push_back({.arg1 = "0", .result = "catch"});

    quadruplets.push_back({.op = "begin", .arg1 = catch_label});
    auto error_symbol = table->lookup(ctx->Identifier()->getText(), false).first;
    quadruplets.push_back({.arg1 = "err", .result = error_symbol.label + error_symbol.name});
    visitBlock(ctx->block().at(1));
    quadruplets.push_back({.op = "end", .arg1 = catch_label});

    return std::any();
}

std::any IRGenerator::visitSwitchStatement(CompiScriptParser::SwitchStatementContext *ctx) {
    auto prev_end = end_label;

    end_label = "l" + std::to_string(label_count + ctx->switchCase().size());    

    auto expr = castSymbol(visitExpression(ctx->expression()));
    auto arg = expr.label + expr.name;
    optimize.push_back({.arg1 = arg, .result = "switch"});
    optimizeQuadruplets();
    temp_count = 0;

    for (auto member: ctx->switchCase())
        visitSwitchCase(member);
    
    if (ctx->defaultCase() != nullptr)
        visitDefaultCase(ctx->defaultCase());

    quadruplets.push_back({.op = "tag", .arg1 = end_label});

    end_label = prev_end;
    return std::any();
}

std::any IRGenerator::visitSwitchCase(CompiScriptParser::SwitchCaseContext *ctx) {
    auto next_label = "l" + std::to_string(label_count++);
    auto expr = castSymbol(visitExpression(ctx->expression()));
    auto arg = expr.value;

    quadruplets.push_back({.op = "==", .arg1 = "switch", .arg2 = arg, .result = "case"});
    quadruplets.push_back({.op = "ifnot", .arg1 = "case", .arg2 = next_label});

    for (auto statement: ctx->statement())
        visitStatement(statement); 

    quadruplets.push_back({.op = "goto", .arg1 = end_label});
    quadruplets.push_back({.op = "tag", .arg1 = next_label});
    return std::any();
}

std::any IRGenerator::visitDefaultCase(CompiScriptParser::DefaultCaseContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitFunctionDeclaration(CompiScriptParser::FunctionDeclarationContext *ctx) {
    auto function = table->lookup(ctx->Identifier()->getText()).first;

    quadruplets.push_back({.op = "begin", .arg1 = function.label + function.name});
    for (auto arg: function.arg_list) {
        quadruplets.push_back({.op = "arg", .arg1 = arg.label + arg.name});
        registry.push_back(arg.label + arg.name);
    }
    
    bool active = func_def;
    func_def = true;
    visitBlock(ctx->block());
    func_def = active;

    quadruplets.push_back({.op = "end", .arg1 = function.label + function.name});
    registry.clear();

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
    auto target = castSymbol(visitLeftHandSide(ctx->lhs));
    auto source = castSymbol(visit(ctx->assignmentExpr()));
    auto arg = (source.type == SymbolType::LITERAL) ? source.value : source.label + source.name;

    optimize.push_back({.arg1 = arg, .result = target.label + target.name});
    return target;
}

std::any IRGenerator::visitPropertyAssignExpr(CompiScriptParser::PropertyAssignExprContext *ctx) {
    auto target = castSymbol(visitLeftHandSide(ctx->lhs));
    auto prop = table->get_property(target.parent, ctx->Identifier()->getText()).first;
    auto source = castSymbol(visit(ctx->assignmentExpr()));
    auto arg = (source.type == SymbolType::LITERAL) ? source.value : source.label + source.name;

    optimize.push_back({.op = "+", .arg1 = target.label + target.name, .arg2 = std::to_string(prop.offset), .result = "i"});
    optimize.push_back({.arg1 = arg, .result = "i*"});

    return prop;
}

std::any IRGenerator::visitExprNoAssign(CompiScriptParser::ExprNoAssignContext *ctx) {
    return visitChildren(ctx);
}

std::any IRGenerator::visitTernaryExpr(CompiScriptParser::TernaryExprContext *ctx) {
    if (!ctx->expression().empty()) {
        auto temp = "t" + std::to_string(temp_count++);
        auto false_label = "l" + std::to_string(label_count++);
        auto end_label = "l" + std::to_string(label_count++);

        auto symbol = castSymbol(visitLogicalOrExpr(ctx->logicalOrExpr()));
        auto arg = (symbol.type == SymbolType::LITERAL) ? symbol.value : symbol.label + symbol.name;

        optimize.push_back({.op = "ifnot", .arg1 = arg, .arg2 = false_label});
        auto expr1 = castSymbol(visitExpression(ctx->expression().at(0)));
        auto arg1 = (expr1.type == SymbolType::LITERAL) ? expr1.value : expr1.label + expr1.name;
        optimize.push_back({.arg1 = arg1, .result = temp});
        optimize.push_back({.op = "goto", .arg1 = end_label});

        optimize.push_back({.op = "tag", .arg1 = false_label});
        auto expr2 = castSymbol(visitExpression(ctx->expression().at(1)));
        auto arg2 = (expr2.type == SymbolType::LITERAL) ? expr2.value : expr2.label + expr2.name;
        optimize.push_back({.arg1 = arg2, .result = temp});
        optimize.push_back({.op = "tag", .arg1 = end_label});

        symbol.name = temp;
        symbol.label = "";
        symbol.type = SymbolType::VARIABLE;
        symbol.data_type = expr1.data_type;
    }
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
    Symbol self;
    for (auto suffixOp: ctx->suffixOp()) {
        auto suffix = castSymbol(visit(suffixOp));
        if (atom.type == SymbolType::FUNCTION && suffix.type == SymbolType::ARGUMENT) {
            for (auto data: registry)
                optimize.push_back({.op = "push", .arg1 = data});

            for (auto &data: suffix.arg_list) {
                auto arg = (data.type == SymbolType::LITERAL) ? data.value : data.label + data.name;
                optimize.push_back({.op = "param", .arg1 = arg});
            }
            if (!self.name.empty()) {
                optimize.push_back({.op = "param", .arg1 = self.label + self.name});
                self.name = "";
            }
             
            optimize.push_back({.op = "call", .arg1 = atom.label + atom.name});

            for (auto data: std::vector(registry.rbegin(), registry.rend()))
                optimize.push_back({.op = "pop", .arg1 = data});

            atom.name = "ret";
            atom.label = "";
            atom.type = SymbolType::VARIABLE;
        }
        else
        if (!atom.parent.empty() && suffix.type == SymbolType::PROPERTY) {
            auto prop = table->get_property(atom.parent, suffix.name).first;
            if (prop.type != SymbolType::FUNCTION) {
                optimize.push_back({.op = "+", .arg1 = atom.label + atom.name, .arg2 = std::to_string(prop.offset), .result = "i"});
                atom = prop;
                atom.name = "i*";
                atom.label = "";
            } else {
                self = atom;
                atom = prop;
            }
        }
        else
        if (!atom.dimentions.empty() && suffix.data_type == SymbolDataType::INTEGER) {
            auto arg = (suffix.type == SymbolType::LITERAL) ? suffix.value : suffix.label + suffix.name;
            // auto temp = "t" + std::to_string(temp_count++);
            optimize.push_back({.arg1 = arg, .result = "t0"});
            optimize.push_back({.op = ">=", .arg1 = "t0", .arg2 = std::to_string(atom.dimentions.at(0)), .result = "err"});
            optimize.push_back({.op = "iferr", .arg1 = "BAD_INDEX"});
            for (int i = 1; i < atom.dimentions.size(); i++) {
                optimize.push_back({.op = "*", .arg1 = "t0", .arg2 = std::to_string(atom.dimentions.at(i)), .result = "t0"});
            }
            optimize.push_back({.op = "*", .arg1 = "t0", .arg2 = std::to_string(getSymbolSize(atom)), .result = "t0"});
            optimize.push_back({.op = "+", .arg1 = atom.label + atom.name, .arg2 = "t0", .result = "i"});

            if (atom.dimentions.size() - 1 > 0) 
                atom.name = "i";
            else 
                atom.name = "i*";

            atom.label = "";
            atom.dimentions = std::vector(atom.dimentions.begin() + 1, atom.dimentions.end());
        }
    }
    return makeAny(atom);
}

std::any IRGenerator::visitIdentifierExpr(CompiScriptParser::IdentifierExprContext *ctx) {
    return table->lookup(ctx->Identifier()->getText(), false).first;
}

std::any IRGenerator::visitNewExpr(CompiScriptParser::NewExprContext *ctx) {
    auto class_symbol = table->lookup(ctx->Identifier()->getText()).first;
    auto constructor = table->get_property(class_symbol.name, "constructor").first;
    auto temp = "t" + std::to_string(temp_count++);

    optimize.push_back({.op = "alloc", .arg1 = std::to_string(class_symbol.size), .result = temp});
    optimize.push_back({.op = "param", .arg1 = temp});
    if (ctx->arguments() != nullptr) {
        auto args = castSymbol(visitArguments(ctx->arguments()));
        for (auto &data :args.arg_list) {
            auto arg = (data.type == SymbolType::LITERAL) ? data.value : data.label + data.name;
            optimize.push_back({.op = "param", .arg1 = arg});
        }
    }
    optimize.push_back({.op = "call", .arg1 = constructor.label + constructor.name});

    Symbol new_symbol = {
        .name = temp,
        .label = "",
        .type = SymbolType::VARIABLE,
    };
    return new_symbol;
}

std::any IRGenerator::visitThisExpr(CompiScriptParser::ThisExprContext *ctx) {
    return table->lookup("this", false).first;
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
    auto name = ctx->Identifier()->getText();
    Symbol symbol_prop = {.name = name, .type = SymbolType::PROPERTY};
    return makeAny(symbol_prop);
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

