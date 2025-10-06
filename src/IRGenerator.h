#pragma once

#include <vector>
#include <string>
#include <stack>

#include "CompiScriptBaseVisitor.h"
#include "SymbolTable.h"

namespace CompiScript {

struct Quad {
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;
};

class IRGenerator: public CompiScriptBaseVisitor
{
private:

    SymbolTable* table;
    std::vector<std::string> registry;
    std::vector<Quad> quadruplets;
    std::vector<Quad> optimize;
    std::string begin_label;
    std::string end_label;
    int temp_count;
    int label_count;
    bool class_def;
    bool func_def;

    void optimizeQuadruplets();

public:
    IRGenerator(SymbolTable *table);
    ~IRGenerator();

    std::string getTAC();

    int getSymbolSize(Symbol symbol);

    std::any visitProgram(CompiScriptParser::ProgramContext *ctx); 


    std::any visitStatement(CompiScriptParser::StatementContext *ctx); 
            

    std::any visitBlock(CompiScriptParser::BlockContext *ctx); 
            

    std::any visitVariableDeclaration(CompiScriptParser::VariableDeclarationContext *ctx); 
            

    std::any visitConstantDeclaration(CompiScriptParser::ConstantDeclarationContext *ctx); 
            

    std::any visitTypeAnnotation(CompiScriptParser::TypeAnnotationContext *ctx); 
            

    std::any visitInitializer(CompiScriptParser::InitializerContext *ctx); 
            

    std::any visitAssignment(CompiScriptParser::AssignmentContext *ctx); 
            

    std::any visitExpressionStatement(CompiScriptParser::ExpressionStatementContext *ctx); 
            

    std::any visitPrintStatement(CompiScriptParser::PrintStatementContext *ctx); 
            

    std::any visitIfStatement(CompiScriptParser::IfStatementContext *ctx); 
            

    std::any visitWhileStatement(CompiScriptParser::WhileStatementContext *ctx); 
            

    std::any visitDoWhileStatement(CompiScriptParser::DoWhileStatementContext *ctx); 
            

    std::any visitForStatement(CompiScriptParser::ForStatementContext *ctx); 
            

    std::any visitForeachStatement(CompiScriptParser::ForeachStatementContext *ctx); 
            

    std::any visitBreakStatement(CompiScriptParser::BreakStatementContext *ctx); 
            

    std::any visitContinueStatement(CompiScriptParser::ContinueStatementContext *ctx); 
            

    std::any visitReturnStatement(CompiScriptParser::ReturnStatementContext *ctx); 
            

    std::any visitTryCatchStatement(CompiScriptParser::TryCatchStatementContext *ctx); 
            

    std::any visitSwitchStatement(CompiScriptParser::SwitchStatementContext *ctx); 
            

    std::any visitSwitchCase(CompiScriptParser::SwitchCaseContext *ctx); 
            

    std::any visitDefaultCase(CompiScriptParser::DefaultCaseContext *ctx); 
            

    std::any visitFunctionDeclaration(CompiScriptParser::FunctionDeclarationContext *ctx); 
            

    std::any visitParameters(CompiScriptParser::ParametersContext *ctx); 
            

    std::any visitParameter(CompiScriptParser::ParameterContext *ctx); 
            

    std::any visitClassDeclaration(CompiScriptParser::ClassDeclarationContext *ctx); 
            

    std::any visitClassMember(CompiScriptParser::ClassMemberContext *ctx); 
            

    std::any visitExpression(CompiScriptParser::ExpressionContext *ctx); 
            

    std::any visitAssignExpr(CompiScriptParser::AssignExprContext *ctx); 
            

    std::any visitPropertyAssignExpr(CompiScriptParser::PropertyAssignExprContext *ctx); 
            

    std::any visitExprNoAssign(CompiScriptParser::ExprNoAssignContext *ctx); 
            

    std::any visitTernaryExpr(CompiScriptParser::TernaryExprContext *ctx); 
            

    std::any visitLogicalOrExpr(CompiScriptParser::LogicalOrExprContext *ctx); 
            

    std::any visitLogicalAndExpr(CompiScriptParser::LogicalAndExprContext *ctx); 
            

    std::any visitEqualityExpr(CompiScriptParser::EqualityExprContext *ctx); 
            

    std::any visitRelationalExpr(CompiScriptParser::RelationalExprContext *ctx); 
            

    std::any visitAdditiveExpr(CompiScriptParser::AdditiveExprContext *ctx); 
            

    std::any visitMultiplicativeExpr(CompiScriptParser::MultiplicativeExprContext *ctx); 
            

    std::any visitUnaryExpr(CompiScriptParser::UnaryExprContext *ctx); 
            

    std::any visitPrimaryExpr(CompiScriptParser::PrimaryExprContext *ctx); 
            

    std::any visitLiteralExpr(CompiScriptParser::LiteralExprContext *ctx); 
            

    std::any visitLeftHandSide(CompiScriptParser::LeftHandSideContext *ctx); 
            

    std::any visitIdentifierExpr(CompiScriptParser::IdentifierExprContext *ctx); 
            

    std::any visitNewExpr(CompiScriptParser::NewExprContext *ctx); 
            

    std::any visitThisExpr(CompiScriptParser::ThisExprContext *ctx); 
            

    std::any visitCallExpr(CompiScriptParser::CallExprContext *ctx); 
            

    std::any visitIndexExpr(CompiScriptParser::IndexExprContext *ctx); 
            

    std::any visitPropertyAccessExpr(CompiScriptParser::PropertyAccessExprContext *ctx); 
            

    std::any visitArguments(CompiScriptParser::ArgumentsContext *ctx); 
            

    std::any visitArrayLiteral(CompiScriptParser::ArrayLiteralContext *ctx); 
            

    std::any visitType(CompiScriptParser::TypeContext *ctx); 
            

    std::any visitBaseType(CompiScriptParser::BaseTypeContext *ctx); 
};

}
