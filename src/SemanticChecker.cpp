#include <string>
#include <print>
#include <regex>
#include <any>

#include "CompiScriptParser.h"
#include "SymbolTable.h"
#include "SemanticChecker.h"

using namespace CompiScript;

// Helper functions

std::any makeAny(Symbol symbol) {
    return std::make_any<Symbol>(symbol);
}

Symbol castSymbol(std::any symbol) {
    return std::any_cast<Symbol>(symbol);
}

SymbolDataType getSymbolDataType(std::string type_name) {
    if (type_name == "integer") return SymbolDataType::INTEGER;
    if (type_name == "string") return SymbolDataType::STRING;
    if (type_name == "boolean") return SymbolDataType::BOOLEAN;
    return SymbolDataType::OBJECT;
}

std::string getSymbolDataTypeString(SymbolDataType type) {
    std::string string_type;
    switch (type) {
        case SymbolDataType::UNDEFINED: string_type = "undefined"; break;
        case SymbolDataType::INTEGER: string_type = "integer"; break;
        case SymbolDataType::BOOLEAN: string_type = "boolean"; break;
        case SymbolDataType::STRING: string_type = "string"; break;
        case SymbolDataType::OBJECT: string_type = "object"; break;
        case SymbolDataType::NIL: string_type = "null"; break;
    }
    return string_type;
}

SemanticChecker::SemanticChecker(): table(), error_count(0), context(Context::NORMAL) {}
SemanticChecker::~SemanticChecker() {}

//SemanticChecker implementations

std::any SemanticChecker::visitProgram(CompiScriptParser::ProgramContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitStatement(CompiScriptParser::StatementContext *ctx) {
    if (ctx->returnStatement() != nullptr)  {
        if (context != Context::FUNCTION)
            std::println("Error: Invalid 'return' outside function.");
        else
            return visitReturnStatement(ctx->returnStatement());
    }
    return visitChildren(ctx);
}

std::any SemanticChecker::visitBlock(CompiScriptParser::BlockContext *ctx) {
    if (context == Context::FUNCTION) {
        std::any symbol_return;
        bool terminate = false;
        for (auto statement: ctx->statement()) {
            if (terminate) {
                std::println("Error: Unreachable code '{}'.", statement->getText().c_str());
                error_count++; continue;
            }

            auto temp = visitStatement(statement);
            if (statement->returnStatement() != nullptr) {
                symbol_return = temp;
                terminate = true;
            }
        }

        return symbol_return;
    }
    return visitChildren(ctx);
}

std::any SemanticChecker::visitVariableDeclaration(CompiScriptParser::VariableDeclarationContext *ctx) {

    auto name = ctx->Identifier()->getText();
    auto exists = table.lookup(name).second;
    if (exists) {
        std::println("Error: '{}' was already defined in this scope.", name.c_str());
        error_count++;
    }

    Symbol new_symbol = {.name = name, .type = SymbolType::VARIABLE };

    if (ctx->typeAnnotation() != nullptr) {
        auto symbol_type = castSymbol(visitTypeAnnotation(ctx->typeAnnotation()));
        new_symbol.data_type = symbol_type.data_type;
        // new_symbol.type = symbol_type.type;
    }

    if (ctx->initializer() != nullptr) {
        auto initiallizer = castSymbol(visitInitializer(ctx->initializer()));
        if (new_symbol.data_type != SymbolDataType::UNDEFINED && new_symbol.data_type != initiallizer.data_type) {
            std::println("Error: Variable '{}' not compatible with value of type '{}'.", 
                         getSymbolDataTypeString(new_symbol.data_type).c_str(),
                         getSymbolDataTypeString(initiallizer.data_type).c_str());
            error_count++;
        }

        new_symbol.value = initiallizer.value;
        new_symbol.data_type = initiallizer.data_type;
    }
    
    table.insert(new_symbol);

    return std::any();
}

std::any SemanticChecker::visitConstantDeclaration(CompiScriptParser::ConstantDeclarationContext *ctx) {

    auto name = ctx->Identifier()->getText();
    auto exists = table.lookup(name).second;
    if (exists) {
        std::println("Error: '{}' was already defined in this scope.", name.c_str());
        error_count++;
    }

    Symbol new_symbol = {.name = name, .type = SymbolType::CONSTANT };

    if (ctx->typeAnnotation() != nullptr) {
        auto symbol_type = castSymbol(visitTypeAnnotation(ctx->typeAnnotation()));
        new_symbol.data_type = symbol_type.data_type;
        // new_symbol.type = symbol_type.type;
    }

    auto expression = castSymbol(visitExpression(ctx->expression()));
    if (new_symbol.data_type != SymbolDataType::UNDEFINED && new_symbol.data_type != expression.data_type) {
        std::println("Error: Constant '{}' not compatible with variable '{}'", 
                     getSymbolDataTypeString(new_symbol.data_type).c_str(),
                     getSymbolDataTypeString(expression.data_type).c_str());
        error_count++;
    }

    new_symbol.value = expression.value;
    new_symbol.data_type = expression.data_type;
    
    table.insert(new_symbol);
    return visitChildren(ctx);
}

std::any SemanticChecker::visitTypeAnnotation(CompiScriptParser::TypeAnnotationContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitInitializer(CompiScriptParser::InitializerContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitAssignment(CompiScriptParser::AssignmentContext *ctx) {
    if (ctx->expression().size() > 1) {
    // TODO
    }
    
    auto name = ctx->Identifier()->getText();
    auto symbol_exists = table.lookup(name);
    if (!symbol_exists.second) {
        std::println("Error: Symbol '{}' isn't defined.", name.c_str());
        error_count++;
    }

    Symbol symbol = symbol_exists.first;
    Symbol expr = castSymbol(visitExpression(ctx->expression().at(0)));
    if (symbol.data_type != expr.data_type) {
        std::println("Error: Type mismatch on assigment");
        error_count++;
    }

    symbol.value = expr.value;
    table.update(name, symbol);

    return makeAny(symbol);
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
    if (ctx->expression() != nullptr)
        return visitExpression(ctx->expression());

    Symbol nil_return = {.data_type = SymbolDataType::NIL};
    return nil_return;
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
    auto name = ctx->Identifier()->getText();
    auto exists = table.lookup(name).second;
    if (exists) {
        std::println("Error: '{}' was already defined in this scope.", name.c_str());
        error_count++;
    }

    Symbol new_symbol = {.name = name, .type = SymbolType::FUNCTION };
    if (ctx->parameters() != nullptr) {
        auto symbol_params = castSymbol(visitParameters(ctx->parameters()));
        new_symbol.arg_list = symbol_params.arg_list;
    }

    if (ctx->type() != nullptr) {
        auto symbol_type = castSymbol(visitType(ctx->type()));
        new_symbol.data_type = symbol_type.data_type;
        if (symbol_type.dimentions > 0) {
            new_symbol.size = symbol_type.size;
            new_symbol.dimentions = symbol_type.dimentions;
        }
    } else {
        new_symbol.data_type = SymbolDataType::NIL;
    }

    table.enter(new_symbol.arg_list);

    bool flag_set = (context & Context::FUNCTION) ? true: false;
    context = (Context)(context | Context::FUNCTION);

    auto symbol_return = castSymbol(visitBlock(ctx->block()));
    new_symbol.definition = table.getCurrent();

    table.exit();
    if (!flag_set)
        context = (Context)(context & ~Context::FUNCTION);

    if (symbol_return.data_type != new_symbol.data_type &&
        symbol_return.size != new_symbol.size &&
        symbol_return.dimentions != new_symbol.dimentions)
    {
        std::println("Error: Invalid return type.");
        error_count++;
    }

    table.insert(new_symbol);
    return std::any();
}

std::any SemanticChecker::visitParameters(CompiScriptParser::ParametersContext *ctx) {
    Symbol symbol_params;
    for (auto param: ctx->parameter()) {
        symbol_params.arg_list.push_back(castSymbol(visitParameter(param)));
    }

    return makeAny(symbol_params);
}

std::any SemanticChecker::visitParameter(CompiScriptParser::ParameterContext *ctx) {
    auto name = ctx->Identifier()->getText();
    Symbol new_symbol = {.name = name, .type = SymbolType::ARGUMENT};
    if (ctx->type() != nullptr) {
        auto symbol_type = castSymbol(visitType(ctx->type()));
        new_symbol.data_type = symbol_type.data_type;
    }
    return makeAny(new_symbol);
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
    if (ctx->expression().size() > 0) {
        auto condition = castSymbol(visitLogicalOrExpr(ctx->logicalOrExpr()));
            if (condition.data_type != SymbolDataType::BOOLEAN) {
                std::println("Error: '{}' is not a boolean type", condition.value.c_str());
            }
        // Type inference later?
        auto symbol_1 = castSymbol(visitExpression(ctx->expression().at(0)));
        auto symbol_2 = castSymbol(visitExpression(ctx->expression().at(1)));
        if (symbol_1.data_type != symbol_2.data_type) {
            std::println("Error: Both expressions in ternary operator must be the same type.");
            error_count++;
        }
        symbol_1.value = "";
        return makeAny(symbol_1);
    }
    return visitChildren(ctx);
}

std::any SemanticChecker::visitLogicalOrExpr(CompiScriptParser::LogicalOrExprContext *ctx) {
    if (ctx->logicalAndExpr().size() > 1) {
        Symbol symbol;
        for (auto operand: ctx->logicalAndExpr()) { 
            symbol = castSymbol(visitLogicalAndExpr(operand));
            if (symbol.data_type != SymbolDataType::BOOLEAN) {
                std::println("Error in '{}': '{}' is not a boolean type", ctx->getText().c_str(), symbol.value.c_str());
                error_count++;
            }
        }
        symbol.value = "";
        return makeAny(symbol);
    }
    return visitChildren(ctx);
}

std::any SemanticChecker::visitLogicalAndExpr(CompiScriptParser::LogicalAndExprContext *ctx) {
    if (ctx->equalityExpr().size() > 1) {
        Symbol symbol;
        for (auto operand: ctx->equalityExpr()) { 
            symbol = castSymbol(visitEqualityExpr(operand));
            if (symbol.data_type != SymbolDataType::BOOLEAN) {
                std::println("Error: '{}' is not a boolean type", symbol.value.c_str());
                error_count++;
            }
        }
        symbol.value = "";
        return makeAny(symbol);
    }
    return visitChildren(ctx);
}

std::any SemanticChecker::visitEqualityExpr(CompiScriptParser::EqualityExprContext *ctx) {
    if (ctx->relationalExpr().size() > 1) {
        auto symbol = castSymbol(visitRelationalExpr(ctx->relationalExpr().at(0)));

        Symbol next_symbol;
        for (int i = 1; i < ctx->relationalExpr().size(); i++) { 
            auto operand = ctx->relationalExpr().at(i);
            std::println("");
            if (symbol.data_type != next_symbol.data_type) {
                std::println("Error: Equality between different types.");
                error_count++;
            }
        }
        symbol.value = "";
        symbol.data_type = SymbolDataType::BOOLEAN;
        return makeAny(symbol);
    }
    return visitChildren(ctx);
}

std::any SemanticChecker::visitRelationalExpr(CompiScriptParser::RelationalExprContext *ctx) {
    if (ctx->additiveExpr().size() > 1) {
        Symbol symbol;
        for (auto operand: ctx->additiveExpr()) { 
            symbol = castSymbol(visitAdditiveExpr(operand));
            if (symbol.data_type != SymbolDataType::INTEGER) {
                std::println("Error: '{}' is not an integer type", symbol.value.c_str());
                error_count++;
            }
        }
        symbol.value = "";
        symbol.data_type = SymbolDataType::BOOLEAN;
        return makeAny(symbol);
    }
    return visitChildren(ctx);
}

std::any SemanticChecker::visitAdditiveExpr(CompiScriptParser::AdditiveExprContext *ctx) {
    if (ctx->multiplicativeExpr().size() > 1) {
        auto symbol = castSymbol(visitMultiplicativeExpr(ctx->multiplicativeExpr().at(0)));
        if (!ctx->SUB().empty() || symbol.data_type != SymbolDataType::STRING) {
            for (auto operand: ctx->multiplicativeExpr()) { 
                symbol = castSymbol(visitMultiplicativeExpr(operand));
                if (symbol.data_type != SymbolDataType::INTEGER) {
                    std::println("Error: '{}' is not an integer type", symbol.value.c_str());
                    error_count++;
                }
            }
        }
        symbol.value = "";
        return makeAny(symbol);
    }
    return visitChildren(ctx);
}

std::any SemanticChecker::visitMultiplicativeExpr(CompiScriptParser::MultiplicativeExprContext *ctx) {
    if (ctx->unaryExpr().size() > 1) {
        Symbol symbol;
        for (auto operand: ctx->unaryExpr()) { 
            symbol = castSymbol(visitUnaryExpr(operand));
            if (symbol.data_type != SymbolDataType::INTEGER) {
                std::println("Error: '{}' is not an integer type", symbol.value.c_str());
                error_count++;
            }
        }
        symbol.value = "";
        return makeAny(symbol);
    }
    return visitChildren(ctx);
}

std::any SemanticChecker::visitUnaryExpr(CompiScriptParser::UnaryExprContext *ctx) {
    if (ctx->unaryExpr() != nullptr) {
        auto unary = ctx->unaryExpr();
        auto op = ctx->getStart()->getText();
        auto symbol = castSymbol(visitUnaryExpr(unary));

        if (op == "!") {
            if (symbol.data_type != SymbolDataType::BOOLEAN) {
                std::println("Error: '{}' is not a boolean type", symbol.value.c_str());
                error_count++;
            } 
        } else {
            if (symbol.data_type != SymbolDataType::INTEGER) {
                std::println("Error: '{}' is not an integer type", symbol.value.c_str());
                error_count++;
            }
        }

        symbol.value = "";
        return makeAny(symbol);
    }

    return visitChildren(ctx);
}

std::any SemanticChecker::visitPrimaryExpr(CompiScriptParser::PrimaryExprContext *ctx) {
    if (ctx->expression() != nullptr) return visitExpression(ctx->expression());
    if (ctx->leftHandSide() != nullptr) return visitLeftHandSide(ctx->leftHandSide());
    return visitLiteralExpr(ctx->literalExpr());
}

std::any SemanticChecker::visitLiteralExpr(CompiScriptParser::LiteralExprContext *ctx) {
    Symbol new_symbol;
    if (ctx->arrayLiteral() != nullptr) {
        // TODO
    }

    new_symbol.value = ctx->getText();
    new_symbol.type = SymbolType::LITERAL;
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

    return makeAny(new_symbol);
}

std::any SemanticChecker::visitLeftHandSide(CompiScriptParser::LeftHandSideContext *ctx) {
    auto atom = castSymbol(visit(ctx->primaryAtom()));
    if (atom.type == SymbolType::FUNCTION) {
        if (ctx->suffixOp().empty()) {
            std::println("Error: Incomplete function call.");
            error_count++;
        }

        auto suffix = castSymbol(visit(ctx->suffixOp().at(0)));
        if (suffix.arg_list.size() != atom.arg_list.size()) {
            std::println("Error: Expected {} arguments, recieved {}.",
                         atom.arg_list.size(),
                         suffix.arg_list.size());
            error_count++;
        }

        int limit = atom.arg_list.size();
        for (int i = 0; i < limit; i++) {
            auto expected = atom.arg_list.at(i).data_type;
            auto received = suffix.arg_list.at(i).data_type;
            if (expected != received) {
                std::println("Error: Expected argument of type '{}', recieved '{}'.",
                             getSymbolDataTypeString(expected),
                             getSymbolDataTypeString(received));
                error_count++;
            }
        }
        // TODO: Manage access to arrays
    }
    return makeAny(atom);
}

std::any SemanticChecker::visitIdentifierExpr(CompiScriptParser::IdentifierExprContext *ctx) {
    auto name = ctx->Identifier()->getText();
    auto symbol_exists = table.lookup(name);
    if (!symbol_exists.second) {
        std::println("Error: '{}' is not defined", name.c_str());
        error_count++;
    }
    return makeAny(symbol_exists.first);
}

std::any SemanticChecker::visitNewExpr(CompiScriptParser::NewExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitThisExpr(CompiScriptParser::ThisExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitCallExpr(CompiScriptParser::CallExprContext *ctx) {
    if (ctx->arguments() == nullptr)
        return Symbol();

    return visitArguments(ctx->arguments());
}

std::any SemanticChecker::visitIndexExpr(CompiScriptParser::IndexExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitPropertyAccessExpr(CompiScriptParser::PropertyAccessExprContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitArguments(CompiScriptParser::ArgumentsContext *ctx) {
    Symbol symbol_arguments;
    for (auto expr: ctx->expression()) {
        symbol_arguments.arg_list.push_back(castSymbol(visitExpression(expr)));
    }
    return makeAny(symbol_arguments);
}

std::any SemanticChecker::visitArrayLiteral(CompiScriptParser::ArrayLiteralContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitType(CompiScriptParser::TypeContext *ctx) {
    auto symbol_type = castSymbol(visitBaseType(ctx->baseType()));
    // TODO: Check if it is an array
    return makeAny(symbol_type);
}

std::any SemanticChecker::visitBaseType(CompiScriptParser::BaseTypeContext *ctx) {
    Symbol symbol_type;
    symbol_type.data_type = getSymbolDataType(ctx->getText());
    // TODO: Check objects
    return makeAny(symbol_type);
}
