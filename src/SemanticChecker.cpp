#include <stdexcept>
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

SemanticChecker::SemanticChecker(): table(), context(TableContext::NORMAL), context_name("") {}
SemanticChecker::~SemanticChecker() {}

//SemanticChecker implementations

std::any SemanticChecker::visitProgram(CompiScriptParser::ProgramContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitStatement(CompiScriptParser::StatementContext *ctx) {
    if (ctx->returnStatement() != nullptr) {
        if (!(context & TableContext::FUNCTION)) {
            std::println(stderr, "Error in line {}: Invalid 'return' outside function.", ctx->getStart()->getLine());
            throw std::runtime_error("INVALID_KEYWORD_USE");
        }
        else {
            return visitReturnStatement(ctx->returnStatement());
        }
    }
    if (ctx->breakStatement() != nullptr) {
        if (!((context & TableContext::WHILE) || (context & TableContext::FOR))) {
            std::println(stderr, "Error in line {}: Invalid use of 'break' keyword.", ctx->getStart()->getLine());
            throw std::runtime_error("INVALID_KEYWORD_USE");
            
        }
    }
    if (ctx->continueStatement() != nullptr) {
        if (!((context & TableContext::WHILE) || (context & TableContext::FOR))) {
            std::println(stderr, "Error in line {}: Invalid use of 'continue' keyword.", ctx->getStart()->getLine());
            throw std::runtime_error("INVALID_KEYWORD_USE");
            
        }
    }
    if (ctx->block() != nullptr) {
        table.enter();
        visitBlock(ctx->block());
        table.exit();
    }
    return visitChildren(ctx);
}

std::any SemanticChecker::visitBlock(CompiScriptParser::BlockContext *ctx) {
    if (context & TableContext::FUNCTION) {
        std::any symbol_return;
        bool terminate = false;
        for (auto statement: ctx->statement()) {
            if (terminate) {
                std::println(stderr, "Error in line {}: Unreachable code '{}'.",
                             ctx->getStart()->getLine(), 
                             statement->getText().c_str());
                throw std::runtime_error("UNREACHEABLE_CODE");
                 continue;
            }

            auto temp = visitStatement(statement);
            if (statement->returnStatement() != nullptr) {
                symbol_return = temp;
                terminate = true;
            }
        }

        if (!symbol_return.has_value()) 
            symbol_return = makeAny({.data_type = SymbolDataType::NIL});

        return symbol_return;
    }
    return visitChildren(ctx);
}

std::any SemanticChecker::visitVariableDeclaration(CompiScriptParser::VariableDeclarationContext *ctx) {

    auto name = ctx->Identifier()->getText();
    auto exists = table.lookup(name).second;
    if (exists) {
        std::println(stderr, "Error in line {}: '{}' was already defined in this scope.", 
                     ctx->getStart()->getLine(),
                     name.c_str());
        throw std::runtime_error("REDEFINITION");
    }

    Symbol new_symbol = {.name = name, .type = SymbolType::VARIABLE };

    if (ctx->typeAnnotation() != nullptr) {
        auto symbol_type = castSymbol(visitTypeAnnotation(ctx->typeAnnotation()));
        new_symbol.label = symbol_type.label;
        new_symbol.data_type = symbol_type.data_type;
        new_symbol.dimentions = symbol_type.dimentions;
    }

    if (ctx->initializer() != nullptr) {
        auto initiallizer = castSymbol(visitInitializer(ctx->initializer()));
        if (new_symbol.data_type != SymbolDataType::UNDEFINED && 
            (new_symbol.data_type != initiallizer.data_type || 
            new_symbol.dimentions != initiallizer.dimentions ||
            new_symbol.label != initiallizer.label)
        ) {
            if (new_symbol.data_type != SymbolDataType::OBJECT) {
                std::println(stderr, "Error in line {}: Variable '{}' not compatible with value of type '{}'.",
                             ctx->getStart()->getLine(), 
                             getSymbolDataTypeString(new_symbol.data_type).c_str(),
                             getSymbolDataTypeString(initiallizer.data_type).c_str());
                throw std::runtime_error("NON_MATCHING_TYPES");
            } else {
                std::println(stderr, "Error in line {}: Variable '{}' not compatible with value of type '{}'.",
                             ctx->getStart()->getLine(), 
                             new_symbol.label.c_str(),
                             initiallizer.label.c_str());
                throw std::runtime_error("NON_MATCHING_TYPES");

            }
        }

        new_symbol.label = initiallizer.label;
        new_symbol.value = initiallizer.value;
        new_symbol.data_type = initiallizer.data_type;
        new_symbol.dimentions = initiallizer.dimentions;
        new_symbol.size = initiallizer.size;
    }

    table.insert(new_symbol);

    return std::any();
}

std::any SemanticChecker::visitConstantDeclaration(CompiScriptParser::ConstantDeclarationContext *ctx) {

    auto name = ctx->Identifier()->getText();
    auto exists = table.lookup(name).second;
    if (exists) {
        std::println(stderr, "Error in line {}: '{}' was already defined in this scope.",
                    ctx->getStart()->getLine(),
                    name.c_str());
        throw std::runtime_error("REDEFINITION");
    }

    Symbol new_symbol = {.name = name, .type = SymbolType::CONSTANT };

    if (ctx->typeAnnotation() != nullptr) {
        auto symbol_type = castSymbol(visitTypeAnnotation(ctx->typeAnnotation()));
        new_symbol.data_type = symbol_type.data_type;
        new_symbol.dimentions = symbol_type.dimentions;
    }

    auto expression = castSymbol(visitExpression(ctx->expression()));
    if (new_symbol.data_type != SymbolDataType::UNDEFINED && 
        (new_symbol.data_type != expression.data_type || 
        new_symbol.dimentions != expression.dimentions ||
        new_symbol.label != expression.label)
    ) {
        if (new_symbol.data_type != SymbolDataType::OBJECT) {
            std::println(stderr, "Error in line {}: Constant '{}' not compatible with value of type '{}'.",
                         ctx->getStart()->getLine(), 
                         getSymbolDataTypeString(new_symbol.data_type).c_str(),
                         getSymbolDataTypeString(expression.data_type).c_str());
            throw std::runtime_error("NON_MATCHING_TYPES");
        } else {
            std::println(stderr, "Error in line {}: Constant '{}' not compatible with value of type '{}'.",
                         ctx->getStart()->getLine(), 
                         new_symbol.label.c_str(),
                         expression.label.c_str());
            throw std::runtime_error("NON_MATCHING_TYPES");
        }
    }

    new_symbol.label = expression.label;
    new_symbol.value = expression.value;
    new_symbol.data_type = expression.data_type;
    new_symbol.dimentions = expression.dimentions;
    new_symbol.size = expression.size;

    table.insert(new_symbol);

    return std::any();
}

std::any SemanticChecker::visitTypeAnnotation(CompiScriptParser::TypeAnnotationContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitInitializer(CompiScriptParser::InitializerContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitAssignment(CompiScriptParser::AssignmentContext *ctx) { 
    auto name = ctx->Identifier()->getText();
    if (ctx->expression().size() > 1) {
        auto symbol = castSymbol(visitExpression(ctx->expression().at(0)));

        if (symbol.data_type != SymbolDataType::OBJECT) {
            std::println(stderr, "Error in line {}: Symbol {} is not of type object.",
                         ctx->getStart()->getLine(),
                         symbol.name.c_str());
            throw std::runtime_error("INVALID_PROPERTY_ACCESS");
        }

        if (symbol.type == SymbolType::CONSTANT) {
            std::println(stderr, "Error in line {}: Can't modify a constant.",
                             ctx->getStart()->getLine());
            throw std::runtime_error("CONSTANT_MODIFICATION");
        }

        auto symbol_exists = table.get_property(symbol.label, name);
        if (!symbol_exists.second) {
            std::println(stderr, "Error in line {}: Property '{}' isn't defined.",
                             ctx->getStart()->getLine(),
                         name.c_str());
            throw std::runtime_error("UNDEFINED_ACCESS");
        }

        auto prop_symbol = symbol_exists.first;
        if (prop_symbol.type == SymbolType::CONSTANT) {
            std::println(stderr, "Error in line {}: Can't modify a constant property.",
                             ctx->getStart()->getLine());
            throw std::runtime_error("CONSTANT_MODIFICATION");
        }
        auto expr = castSymbol(visitExpression(ctx->expression().at(1)));
        if (prop_symbol.data_type != expr.data_type || prop_symbol.dimentions != expr.dimentions) {
            std::println(stderr, "Error in line {}: Type mismatch on assigment",
                             ctx->getStart()->getLine());
            throw std::runtime_error("NON_MATCHING_TYPES");
        }

        // prop_symbol.value = expr.value;
        // table.set_property(symbol.label, name, prop_symbol);
        return makeAny(symbol);
    }

    auto symbol_exists = table.lookup(name, false);
    if (!symbol_exists.second) {
        std::println(stderr, "Error in line {}: Symbol '{}' isn't defined.",
                             ctx->getStart()->getLine(),
                     name.c_str());
        throw std::runtime_error("UNDEFINED_ACCESS");
    }

    Symbol symbol = symbol_exists.first;
    if (symbol.type == SymbolType::CONSTANT) {
        std::println(stderr, "Error in line {}: Can't modify a constant.",
                             ctx->getStart()->getLine());
        throw std::runtime_error("CONSTANT_MODIFICATION");
    }
    Symbol expr = castSymbol(visitExpression(ctx->expression().at(0)));
    if (symbol.data_type != expr.data_type || symbol.dimentions != expr.dimentions) {
        std::println(stderr, "Error in line {}: Type mismatch on assigment.",
                             ctx->getStart()->getLine());
        throw std::runtime_error("NON_MATCHING_TYPES");
    }

    symbol.value = expr.value;
    table.update(name, symbol);

    return makeAny(symbol);
}

std::any SemanticChecker::visitExpressionStatement(CompiScriptParser::ExpressionStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitPrintStatement(CompiScriptParser::PrintStatementContext *ctx) {
    auto symbol = castSymbol(visitExpression(ctx->expression()));
    if (symbol.data_type == SymbolDataType::OBJECT ||
        symbol.data_type == SymbolDataType::NIL || 
        symbol.data_type == SymbolDataType::UNDEFINED) 
    {
        std::println(stderr, "Error in line {}: Can't print symbol of type '{}'.",
                             ctx->getStart()->getLine(), 
                     getSymbolDataTypeString(symbol.data_type).c_str());
        throw std::runtime_error("INVALID_TYPE");
    }
    return visitChildren(ctx);
}

std::any SemanticChecker::visitIfStatement(CompiScriptParser::IfStatementContext *ctx) {
    auto condition = castSymbol(visitExpression(ctx->expression()));
    if (condition.data_type != SymbolDataType::BOOLEAN) {
        auto symbol_str = (condition.type == SymbolType::LITERAL) ? 
            condition.value.c_str() : 
            condition.name.c_str();
        std::println(stderr, "Error in line {}: '{}' is not a boolean type",
                             ctx->getStart()->getLine(),
                     symbol_str);
        throw std::runtime_error("INVALID_TYPE");
    }

    for (auto block :ctx->block()) {
        table.enter();
        visitBlock(block);
        table.exit();
    }

    return std::any();
}

std::any SemanticChecker::visitWhileStatement(CompiScriptParser::WhileStatementContext *ctx) {
    auto condition = castSymbol(visitExpression(ctx->expression()));
    if (condition.data_type != SymbolDataType::BOOLEAN) {
        auto symbol_str = (condition.type == SymbolType::LITERAL) ? 
            condition.value.c_str() : 
            condition.name.c_str();
        std::println(stderr, "Error in line {}: '{}' is not a boolean type",
                             ctx->getStart()->getLine(), symbol_str);
        throw std::runtime_error("INVALID_TYPE");    
    }

    bool flag_set = (context & TableContext::WHILE) ? true: false;
    context = (TableContext)(context | TableContext::WHILE);

    table.enter();
    visitBlock(ctx->block());
    table.exit();

    if (!flag_set)
        context = (TableContext)(context & ~TableContext::WHILE);

    return std::any();
}

std::any SemanticChecker::visitDoWhileStatement(CompiScriptParser::DoWhileStatementContext *ctx) {
    auto condition = castSymbol(visitExpression(ctx->expression()));
    if (condition.data_type != SymbolDataType::BOOLEAN) {
        auto symbol_str = (condition.type == SymbolType::LITERAL) ? 
            condition.value.c_str() : 
            condition.name.c_str();
        std::println(stderr, "Error in line {}: '{}' is not a boolean type",
                             ctx->getStart()->getLine(),
                     symbol_str);
        throw std::runtime_error("INVALID_TYPE");
    }

    bool flag_set = (context & TableContext::WHILE) ? true: false;
    context = (TableContext)(context | TableContext::WHILE);

    table.enter();
    visitBlock(ctx->block());
    table.exit();

    if (!flag_set)
        context = (TableContext)(context & ~TableContext::WHILE);

    return std::any();
}

std::any SemanticChecker::visitForStatement(CompiScriptParser::ForStatementContext *ctx) {
    bool flag_set = (context & TableContext::FOR) ? true: false;
    context = (TableContext)(context | TableContext::FOR);
    if (ctx->assignment() != nullptr) 
        visitAssignment(ctx->assignment());

    table.enter();
    if (ctx->variableDeclaration() != nullptr)
        visitVariableDeclaration(ctx->variableDeclaration());

    if (ctx->expression().at(0) != nullptr) {
        auto condition = castSymbol(visitExpression(ctx->expression().at(0)));
        if (condition.data_type != SymbolDataType::BOOLEAN) {
            auto symbol_str = (condition.type == SymbolType::LITERAL) ? 
                condition.value.c_str() : 
                condition.name.c_str();
            std::println(stderr, "Error in line {}: '{}' is not a boolean type",
                             ctx->getStart()->getLine(), symbol_str);
            throw std::runtime_error("INVALID_TYPE");
        }
    }

    if (ctx->expression().at(1) != nullptr)
        visitExpression(ctx->expression().at(1));

    visitBlock(ctx->block());
    table.exit();

    if (!flag_set)
        context = (TableContext)(context & ~TableContext::FOR);

    return std::any();
}

std::any SemanticChecker::visitForeachStatement(CompiScriptParser::ForeachStatementContext *ctx) {
    auto iter_symbol = castSymbol(visitExpression(ctx->expression()));
    if (iter_symbol.dimentions == 0) {
        std::println(stderr, "Error in line {}: For-each loop can't iterate over non array type.",
                             ctx->getStart()->getLine());
        throw std::runtime_error("INVALID_TYPE");
    }

    Symbol new_symbol = {
        .name = ctx->Identifier()->getText(),
        .label = iter_symbol.label,
        .type = SymbolType::VARIABLE,
        .data_type = iter_symbol.data_type,
        .dimentions = iter_symbol.dimentions - 1,
    };

    bool flag_set = (context & TableContext::FOR) ? true: false;
    context = (TableContext)(context | TableContext::FOR);

    table.enter({new_symbol});
    visitBlock(ctx->block());
    table.exit();

    if (!flag_set)
        context = (TableContext)(context & ~TableContext::FOR);

    return std::any();
}

std::any SemanticChecker::visitBreakStatement(CompiScriptParser::BreakStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitContinueStatement(CompiScriptParser::ContinueStatementContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitReturnStatement(CompiScriptParser::ReturnStatementContext *ctx) {
    if (ctx->expression() != nullptr) {
        auto func_symbol = table.lookup(context_name, false).first;
        auto symbol_return = castSymbol(visitExpression(ctx->expression()));

        if (func_symbol.data_type != SymbolDataType::NIL && (
            symbol_return.data_type != func_symbol.data_type ||
            symbol_return.dimentions != func_symbol.dimentions))
        {
            std::println(stderr, "Error in line {}: Invalid return type.",
                             ctx->getStart()->getLine());
            throw std::runtime_error("INVALID_TYPE");
            
        }

        return symbol_return;
    }

    Symbol nil_return = {.data_type = SymbolDataType::NIL};
    return nil_return;
}

std::any SemanticChecker::visitTryCatchStatement(CompiScriptParser::TryCatchStatementContext *ctx) {
    table.enter();
    visitBlock(ctx->block().at(0));
    table.exit();

    Symbol error_symbol = {
        .type = SymbolType::CONSTANT,
        .data_type = SymbolDataType::STRING,
    };

    table.enter({error_symbol});
    visitBlock(ctx->block().at(1));
    table.exit();

    return std::any();
}

std::any SemanticChecker::visitSwitchStatement(CompiScriptParser::SwitchStatementContext *ctx) {
    auto condition = castSymbol(visitExpression(ctx->expression()));
    if (condition.data_type == SymbolDataType::OBJECT) {
        std::println(stderr, "Error in line {}: Can't switch a type 'OBJECT' expresion.",
                             ctx->getStart()->getLine());
        throw std::runtime_error("INVALID_TYPE");
         
    }
    for (auto s_case: ctx->switchCase()) {
        auto case_symbol = castSymbol(visitSwitchCase(s_case));
        if (case_symbol.data_type != condition.data_type) {
            std::println(stderr, "Error in line {}: Case of type {} doesn't match condition of type {}.",
                             ctx->getStart()->getLine(),
                         getSymbolDataTypeString(case_symbol.data_type),
                         getSymbolDataTypeString(condition.data_type));
            throw std::runtime_error("INVALID_TYPE");
            
        }
    }

    if (ctx->defaultCase() != nullptr)
        visitDefaultCase(ctx->defaultCase());

    return std::any();
}

std::any SemanticChecker::visitSwitchCase(CompiScriptParser::SwitchCaseContext *ctx) {
    auto case_symbol = castSymbol(visitExpression(ctx->expression()));
    if (case_symbol.type != SymbolType::LITERAL) {
        std::println(stderr, "Error in line {}: Case expression must be of type 'LITERAL'.",
                             ctx->getStart()->getLine());
        throw std::runtime_error("INVALID_TYPE");
        
    }

    table.enter();

    for (auto statement: ctx->statement())
    visitStatement(statement);

    table.exit();
    return makeAny(case_symbol);
}

std::any SemanticChecker::visitDefaultCase(CompiScriptParser::DefaultCaseContext *ctx) {
    table.enter();

    for (auto statement: ctx->statement())
    visitStatement(statement);

    table.exit();
    return std::any();
}

std::any SemanticChecker::visitFunctionDeclaration(CompiScriptParser::FunctionDeclarationContext *ctx) {
    auto name = ctx->Identifier()->getText();
    auto exists = table.lookup(name).second;
    if (exists) {
        std::println(stderr, "Error in line {}: '{}' was already defined in this scope.",
                             ctx->getStart()->getLine(),
                     name.c_str());
        throw std::runtime_error("REDEFINITION");
        
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
            new_symbol.dimentions = symbol_type.dimentions;
        }
    } else {
        new_symbol.data_type = SymbolDataType::NIL;
    }

    table.insert(new_symbol);

    table.enter(new_symbol.arg_list);

    auto prev_context_name = context_name;
    context_name = name;

    bool flag_set = (context & TableContext::FUNCTION) ? true: false;
    context = (TableContext)(context | TableContext::FUNCTION);

    auto symbol_return = castSymbol(visitBlock(ctx->block()));
    if (symbol_return.data_type == SymbolDataType::NIL && new_symbol.data_type != SymbolDataType::NIL) {
        std::println(stderr, "Error in line {}: The function must return a value of type '{}'.",
                             ctx->getStart()->getLine(), 
                     getSymbolDataTypeString(new_symbol.data_type));
        throw std::runtime_error("MISSING_RETURN");
        
    }
    new_symbol.definition = table.getCurrent();
    table.update(name, new_symbol);

    if (!flag_set)
        context = (TableContext)(context & ~TableContext::FUNCTION);
    table.exit();

    context_name = prev_context_name;

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
    auto name = ctx->Identifier().at(0)->getText();
    auto exists = table.lookup(name).second;
    if (exists) {
        std::println(stderr, "Error in line {}: '{}' was already defined in this scope.",
                             ctx->getStart()->getLine(),
                     name.c_str());
        throw std::runtime_error("REDEFINITION");
        
    }

    Symbol new_symbol = {.name = name, .type = SymbolType::CLASS };
    if (ctx->Identifier().size() > 1) {
        auto label = ctx->Identifier().at(1)->getText();
        auto symbol_exists = table.lookup(label, false);
        if (!symbol_exists.second) {
            std::println(stderr, "Error in line {}: parent class '{}' does not exist.",
                             ctx->getStart()->getLine(),
                         name.c_str());
            throw std::runtime_error("UNDEFINED_ACCESS");
            
        }
        new_symbol.label = label;
        new_symbol.arg_list = symbol_exists.first.arg_list;
    }

    table.insert(new_symbol);

    table.enter();
    context = (TableContext)(context | TableContext::CLASS);

    new_symbol.definition = table.getCurrent();
    table.update(name, new_symbol);

    Symbol symbol_self = {
        .name = "this", 
        .label = name,
        .type = SymbolType::VARIABLE, 
        .data_type = SymbolDataType::OBJECT, 
    };
    table.insert(symbol_self);

    for (auto member: ctx->classMember())
    visitClassMember(member);

    if (table.lookup("constructor").second) {
        auto constructor = table.lookup("constructor").first;
        new_symbol.arg_list = constructor.arg_list;
        table.update(name, new_symbol);
    }

    context = (TableContext)(context & ~TableContext::CLASS);
    table.exit();

    return std::any();
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
    auto prop_name = ctx->Identifier()->getText();
    auto symbol = castSymbol(visitLeftHandSide(ctx->lhs));

    if (symbol.data_type != SymbolDataType::OBJECT) {
        std::println(stderr, "Error in line {}: Symbol {} is not of type object.",
                             ctx->getStart()->getLine(),
                     symbol.name.c_str());
        throw std::runtime_error("INVALID_PROPERTY_ACCESS");
        
    }

    auto symbol_exists = table.get_property(symbol.label, prop_name);
    if (!symbol_exists.second) {
        std::println(stderr, "Error in line {}: Property '{}' isn't defined.",
                             ctx->getStart()->getLine(),
                     prop_name.c_str());
        throw std::runtime_error("UNDEFINED_ACCESS");
        
    }

    auto prop_symbol = symbol_exists.first;
    auto expr = castSymbol(visit(ctx->assignmentExpr()));
    if (prop_symbol.data_type != expr.data_type || prop_symbol.dimentions != expr.dimentions) {
        std::println(stderr, "Error in line {}: Type mismatch on assigment",
                             ctx->getStart()->getLine());
        throw std::runtime_error("NON_MATCHING_TYPES");
    }

    // prop_symbol.value = expr.value;
    // table.set_property(symbol.label, prop_name, prop_symbol);

    return makeAny(prop_symbol);
}

std::any SemanticChecker::visitExprNoAssign(CompiScriptParser::ExprNoAssignContext *ctx) {
    return visitChildren(ctx);
}

std::any SemanticChecker::visitTernaryExpr(CompiScriptParser::TernaryExprContext *ctx) {
    if (ctx->expression().size() > 0) {
        auto condition = castSymbol(visitLogicalOrExpr(ctx->logicalOrExpr()));
        if (condition.data_type != SymbolDataType::BOOLEAN) {
            auto symbol_str = (condition.type == SymbolType::LITERAL) ? 
                condition.value.c_str() : 
                condition.name.c_str();
            std::println(stderr, "Error in line {}: '{}' is not a boolean type",
                             ctx->getStart()->getLine(),
                         symbol_str);
            throw std::runtime_error("NON_MATCHING_TYPES");
        }
        // Type inference later?
        auto symbol_1 = castSymbol(visitExpression(ctx->expression().at(0)));
        auto symbol_2 = castSymbol(visitExpression(ctx->expression().at(1)));
        if (symbol_1.data_type != symbol_2.data_type) {
            std::println(stderr, "Error in line {}: Both expressions in ternary operator must be the same type.",
                             ctx->getStart()->getLine());
            throw std::runtime_error("NON_MATCHING_TYPES");
            
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
                std::println(stderr, "Error in line {}: '{}' is not a boolean type",
                             ctx->getStart()->getLine(),
                             symbol.value.c_str());
                throw std::runtime_error("NON_MATCHING_TYPES");
                
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
                auto symbol_str = (symbol.type == SymbolType::LITERAL) ? 
                    symbol.value.c_str() : 
                    symbol.name.c_str();
                std::println(stderr, "Error in line {}: '{}' is not a boolean type",
                             ctx->getStart()->getLine(),
                             symbol_str);
                throw std::runtime_error("NON_MATCHING_TYPES");
                
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
            next_symbol = castSymbol(visitRelationalExpr(ctx->relationalExpr().at(i)));
            if (symbol.data_type != next_symbol.data_type) {
                std::println(stderr, "Error in line {}: Equality between different types.",
                             ctx->getStart()->getLine());
                throw std::runtime_error("NON_MATCHING_TYPES");
                
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
                auto symbol_str = (symbol.type == SymbolType::LITERAL) ? 
                    symbol.value.c_str() : 
                    symbol.name.c_str();
                std::println(stderr, "Error in line {}: '{}' is not an integer type",
                             ctx->getStart()->getLine(),
                             symbol_str);
                throw std::runtime_error("NON_MATCHING_TYPES");
                
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
                    auto symbol_str = (symbol.type == SymbolType::LITERAL) ? 
                        symbol.value.c_str() : 
                        symbol.name.c_str();
                    std::println(stderr, "Error in line {}: '{}' is not an integer type",
                             ctx->getStart()->getLine(),
                                 symbol_str);
                    throw std::runtime_error("NON_MATCHING_TYPES");
                    
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
                auto symbol_str = (symbol.type == SymbolType::LITERAL) ? 
                    symbol.value.c_str() : 
                    symbol.name.c_str();
                std::println(stderr, "Error in line {}: '{}' is not an integer type",
                             ctx->getStart()->getLine(),
                             symbol_str);
                throw std::runtime_error("NON_MATCHING_TYPES");
                
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
                auto symbol_str = (symbol.type == SymbolType::LITERAL) ? 
                    symbol.value.c_str() : 
                    symbol.name.c_str();
                std::println(stderr, "Error in line {}: '{}' is not a boolean type",
                             ctx->getStart()->getLine(),
                             symbol_str);
                throw std::runtime_error("NON_MATCHING_TYPES");
                
            } 
        } else {
            if (symbol.data_type != SymbolDataType::INTEGER) {
                auto symbol_str = (symbol.type == SymbolType::LITERAL) ? 
                    symbol.value.c_str() : 
                    symbol.name.c_str();
                std::println(stderr, "Error in line {}: '{}' is not an integer type",
                             ctx->getStart()->getLine(),
                             symbol_str);
                throw std::runtime_error("NON_MATCHING_TYPES");
                
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

std::any SemanticChecker::visitLeftHandSide(CompiScriptParser::LeftHandSideContext *ctx) {
    auto atom = castSymbol(visit(ctx->primaryAtom()));

    if (atom.type == SymbolType::FUNCTION && ctx->suffixOp().empty()) {
        std::println(stderr, "Error in line {}: Incomplete function call.",
                             ctx->getStart()->getLine());
        throw std::runtime_error("INCOMPLETE_CALL");
        
    }

    for (auto suffixOp: ctx->suffixOp()) {
        // TODO: Check other Suffixes
        auto suffix = castSymbol(visit(suffixOp));
        if (atom.type == SymbolType::FUNCTION && suffix.type == SymbolType::ARGUMENT) {
            if (suffix.arg_list.size() != atom.arg_list.size()) {
                std::println(stderr, "Error in line {}: Expected {} arguments, recieved {}.",
                             ctx->getStart()->getLine(),
                             atom.arg_list.size(),
                             suffix.arg_list.size());
                throw std::runtime_error("INCOMPLETE_CALL");
            }

            int limit = atom.arg_list.size();
            for (int i = 0; i < limit; i++) {
                auto expected = atom.arg_list.at(i).data_type;
                auto received = suffix.arg_list.at(i).data_type;
                if (expected != received) {
                    std::println(stderr, "Error in line {}: Expected argument of type '{}', recieved '{}'.",
                             ctx->getStart()->getLine(),
                                 getSymbolDataTypeString(expected),
                                 getSymbolDataTypeString(received));
                    throw std::runtime_error("NON_MATCHING_TYPES");
                }
            }
        }
        else
        if (atom.dimentions > 0 && suffix.data_type == SymbolDataType::INTEGER) {
            atom.dimentions--;
        }
        else
        if (!atom.label.empty() && suffix.type == SymbolType::PROPERTY) {
            auto prop_exists = table.get_property(atom.label, suffix.name);
            if (!prop_exists.second) {
                std::println(stderr, "Error in line {}: Property doesn't exist.",
                             ctx->getStart()->getLine());
                throw std::runtime_error("UNDEFINED_ACCESS");
            }
            atom = prop_exists.first;
        }
        else
        {
            std::println(stderr, "Error in line {}: Invalid suffix.",
                             ctx->getStart()->getLine());
            throw std::runtime_error("INVALID_SUFFIX");
        }
    }

    return makeAny(atom);
}

std::any SemanticChecker::visitIdentifierExpr(CompiScriptParser::IdentifierExprContext *ctx) {
    auto name = ctx->Identifier()->getText();
    auto symbol_exists = table.lookup(name, false);
    if (!symbol_exists.second) {
        std::println(stderr, "Error in {}: '{}' is not defined",
                             ctx->getStart()->getLine(),
                     name.c_str());
        throw std::runtime_error("UNDEFINED_ACCESS");
        
    }
    return makeAny(symbol_exists.first);
}

std::any SemanticChecker::visitNewExpr(CompiScriptParser::NewExprContext *ctx) {
    auto name = ctx->Identifier()->getText();
    auto symbol_exists = table.lookup(name, false);
    if (!symbol_exists.second) {
        std::println(stderr, "Error in line {}: '{}' is not defined",
                             ctx->getStart()->getLine(),
                     name.c_str());
        throw std::runtime_error("UNDEFINED_ACCESS");
        
    }

    auto class_symbol = symbol_exists.first;
    if (class_symbol.type != SymbolType::CLASS) {
        std::println(stderr, "Error in line {}: '{}' is not a class",
                             ctx->getStart()->getLine(),
                     name.c_str());
        throw std::runtime_error("NON_MATCHIN_TYPES");
        
    }

    if (ctx->arguments() != nullptr) {
        auto args_symbol = castSymbol(visitArguments(ctx->arguments()));
        if (class_symbol.arg_list.size() != args_symbol.arg_list.size()) {
            std::println(stderr, "Error in line {}: Expected {} arguments, recieved {}.",
                             ctx->getStart()->getLine(),
                         class_symbol.arg_list.size(),
                         args_symbol.arg_list.size());
            throw std::runtime_error("NON_MATCHING_ARGUMENTS");

        }

        int limit = class_symbol.arg_list.size();
        for (int i = 0; i < limit; i++) {
            auto expected = class_symbol.arg_list.at(i).data_type;
            auto received = args_symbol.arg_list.at(i).data_type;
            if (expected != received) {
                std::println(stderr, "Error in line {}: Expected argument of type '{}', recieved '{}'.",
                             ctx->getStart()->getLine(),
                             getSymbolDataTypeString(expected),
                             getSymbolDataTypeString(received));
                throw std::runtime_error("NON_MATCHING_TYPES");

            }
        }        
    }

    auto new_symbol = Symbol{
        .name = name,
        .label = class_symbol.name,
        .data_type = SymbolDataType::OBJECT,
    };
    return makeAny(new_symbol);
}

std::any SemanticChecker::visitThisExpr(CompiScriptParser::ThisExprContext *ctx) {
    auto symbol_exists = table.lookup("this", false);
    if (!symbol_exists.second) {
        std::println(stderr, "Error in line {}: Invalid use of reserved word 'this'",
                             ctx->getStart()->getLine());
        throw std::runtime_error("INVALID_KEYWORD_USE");
        
    }
    return makeAny(symbol_exists.first);
}

std::any SemanticChecker::visitCallExpr(CompiScriptParser::CallExprContext *ctx) {
    if (ctx->arguments() == nullptr)
        return Symbol({.type = SymbolType::ARGUMENT});

    return visitArguments(ctx->arguments());
}

std::any SemanticChecker::visitIndexExpr(CompiScriptParser::IndexExprContext *ctx) {
    Symbol array_index = castSymbol(visitExpression(ctx->expression()));
    if (array_index.data_type != SymbolDataType::INTEGER)
    {
        std::println(stderr, "Error in line {}: Invalid index value.",
                             ctx->getStart()->getLine());
        throw std::runtime_error("INVALID_INDEX");
        array_index.data_type = SymbolDataType::INTEGER;
        array_index.value = "0";
        
    }
    return array_index;
}

std::any SemanticChecker::visitPropertyAccessExpr(CompiScriptParser::PropertyAccessExprContext *ctx) {
    auto name = ctx->Identifier()->getText();
    Symbol symbol_prop = {.name = name, .type = SymbolType::PROPERTY};
    return makeAny(symbol_prop);
}

std::any SemanticChecker::visitArguments(CompiScriptParser::ArgumentsContext *ctx) {
    Symbol symbol_arguments = {.type = SymbolType::ARGUMENT};
    for (auto expr: ctx->expression()) {
        symbol_arguments.arg_list.push_back(castSymbol(visitExpression(expr)));
    }
    return makeAny(symbol_arguments);
}

std::any SemanticChecker::visitArrayLiteral(CompiScriptParser::ArrayLiteralContext *ctx) {
    Symbol array_symbol = {.data_type = SymbolDataType::UNDEFINED};
    // TODO: let id = [] case
    Symbol comparison = {};
    if (!ctx->expression().empty()) {
        comparison = castSymbol(visitExpression(ctx->expression().at(0)));
        array_symbol.data_type = comparison.data_type;
        array_symbol.dimentions = comparison.dimentions;
    }

    for (auto expr: ctx->expression()) {
        auto value_symbol = castSymbol(visitExpression(expr));
        if (value_symbol.data_type != comparison.data_type || 
            value_symbol.size != comparison.size || 
            value_symbol.dimentions != comparison.dimentions
        ) {
            std::println(stderr, "Error in line {}: Non matching data types in array literal",
                             ctx->getStart()->getLine());
            throw std::runtime_error("NON_MATCHING_TYPES");
            
        }

        array_symbol.value.append(value_symbol.value + ';');
        array_symbol.size += value_symbol.size; 
    }

    array_symbol.dimentions++;
    return makeAny(array_symbol);
}

std::any SemanticChecker::visitType(CompiScriptParser::TypeContext *ctx) {
    auto symbol_type = castSymbol(visitBaseType(ctx->baseType()));
    auto token_string = ctx->getText();
    while (token_string.ends_with("[]")) {
        token_string.erase(token_string.length() - 2);
        symbol_type.dimentions++;
    }
    return makeAny(symbol_type);
}

std::any SemanticChecker::visitBaseType(CompiScriptParser::BaseTypeContext *ctx) {
    Symbol symbol_type;
    symbol_type.data_type = getSymbolDataType(ctx->getText());
    if (symbol_type.data_type == SymbolDataType::OBJECT) { 
        auto symbol_exists = table.lookup(ctx->getText(), false);
        if (!symbol_exists.second) {
            std::println(stderr, "Error in line {}: '{}' is not defined",
                             ctx->getStart()->getLine(),
                         ctx->getText().c_str());
            throw std::runtime_error("UNDEFINED_ACCESS");
            
        }

        auto class_symbol = symbol_exists.first;
        if (class_symbol.type != SymbolType::CLASS) {
            std::println(stderr, "Error in line {}: '{}' is not a class",
                             ctx->getStart()->getLine(),
                         ctx->getText().c_str());
            throw std::runtime_error("NON_MATCHING_TYPES");
            
        }

        symbol_type.label = class_symbol.name;
    }
    return makeAny(symbol_type);
}
