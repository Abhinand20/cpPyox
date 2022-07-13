#pragma once

#include<iostream>
#include<sstream>
#include<string>
#include"../scanner/Expr.h"
#include"../scanner/token.h"
#include"../utils/runtimeError.h"
#include"environment.h"
#include<type_traits>
#include<any>

/*
Scanner and Parser together will create Abstract Syntax Trees according to the grammar

Interpreter follows visitor pattern and computes all expressions in the tree by visiting nodes in a post-order fashion (L->R->Node)

*/

class Interpreter : public ExprVisitor, public StmtVisitor {

public:
    void interpret(std::vector<std::shared_ptr<Stmt>>& statements){
        try {
            for(const std::shared_ptr<Stmt>& statement : statements){
                execute(statement);
            }
        }
        catch (RuntimeError error){
            runtimeError(error);
        }
    }

    std::any visitBlockStmt(std::shared_ptr<Block> stmt) override {
        // Create a new environment with the current one as enclosing (For nesting/shadowing)
        executeBlock(stmt->statements, std::make_shared<Environment>(environment) );
        return {};
    }

    std::any visitIfStmt(std::shared_ptr<If> stmt) override {
        if(isTruthy(evaluate(stmt->condition))) {
            execute(stmt->thenBranch);
        } else if (stmt->elseBranch != nullptr) {
            execute(stmt->elseBranch);
        }

        return {};
    }

    std::any visitExpressionStmt(std::shared_ptr<Expression> stmt) override {
        // Statements do not produce values, so we evaluate the expression and dont return anythin
        evaluate(stmt->expression);
        return {};
    }

    std::any visitPrintStmt(std::shared_ptr<Print> stmt) override {
        // Print the evaluated expression
        std::any value = evaluate(stmt->expression);
        std::cout << stringify(value) << std::endl;
        return {};
    }

    // Evaluate and store a variable declaration
    std::any visitVarStmt(std::shared_ptr<Var> stmt) override {
        std::any value = nullptr;
        
        if(stmt->initializer != nullptr) {
            value = evaluate(stmt->initializer);
        }
        environment->define(stmt->name.lexeme,std::move(value));

        return {};
    }

    // Evaluate while control flow
    std::any visitWhileStmt(std::shared_ptr<While> stmt) override {
        while(isTruthy(evaluate(stmt->condition))) {
            execute(stmt->body);
        }

        return {};
    }

    // Evaluate assignment statements
    std::any visitAssignExpr(std::shared_ptr<Assign> expr) override {
        std::any value = evaluate(expr->value);
        environment->assign(expr->name,value);
        
        return value;
    }


    // Evaluate Literals : directly return value
    std::any visitLiteralExpr(std::shared_ptr<Literal> expr) override {
        return expr->value; // Check if we need to type_cast this before returning
    }

    std::any visitLogicalExpr(std::shared_ptr<Logical> expr) override {
        std::any left = evaluate(expr->left);

        if(expr->op.type == OR){
            if(isTruthy(left)) return left; // left gives true and if its ||, we return left (true)
        } else {
            if(!isTruthy(left)) return left; // left gives false and if its &&, we return left (false)
        }

        return evaluate(expr->right); // Return right finally only if all left conditions are met
    }

    // Evaluate parentheses : recursively evaluate the expression inside 
    std::any visitGroupingExpr(std::shared_ptr<Grouping> expr) override {
        return evaluate(expr->expression);
    }

    // Evaluate unary expressions
    std::any visitUnaryExpr(std::shared_ptr<Unary> expr) override {
        // Evaluate the expression on right 
        std::any right = evaluate(expr->right);

        // Proceed further according to the operator type
        switch(expr->op.type){
            case(MINUS): {
                checkNumberOperand(expr->op,right); // Check type before casting
                return -std::any_cast<double>(right);
                }
            case(BANG) : {
                return !isTruthy(right);
                }
            default: break;
        }

        return nullptr;
    }

    // Get the value of variable from lookup table
    std::any visitVariableExpr(std::shared_ptr<Variable> expr) override {
        return environment->get(expr->name);
    }

    // Evaluate binary operations
    std::any visitBinaryExpr(std::shared_ptr<Binary> expr) override {
        // Evaluates operands from left -> right
        std::any left = evaluate(expr->left);
        std::any right = evaluate(expr->right);

        switch(expr->op.type){
            
            ////  ARITHMETIC OPERATORS  ////
            case(MINUS): {
                checkNumberOperand(expr->op,left,right);
                return std::any_cast<double>(left) - std::any_cast<double>(right);
            }
            
            case(PLUS):{
                if(left.type() == typeid(std::string) && right.type() == typeid(std::string)) {
                    return std::any_cast<std::string>(left) + std::any_cast<std::string>(right);
                }
                if(left.type() == typeid(double) && right.type() == typeid(double)) {
                    return std::any_cast<double>(left) + std::any_cast<double>(right);
                }
                throw RuntimeError(expr->op, "Operands must be two numbers or two strings.");
            }
            
            case(SLASH): {
                checkNumberOperand(expr->op,left,right);
                return std::any_cast<double>(left) / std::any_cast<double>(right);
            }
            
            case(STAR): {
                checkNumberOperand(expr->op,left,right);
                return std::any_cast<double>(left) * std::any_cast<double>(right);
            }

            ////  COMPARISION OPERATORS  ////
            case(GREATER):{
                checkNumberOperand(expr->op,left,right);
                return std::any_cast<double>(left) > std::any_cast<double>(right);
            }

            case(GREATER_EQUAL):{
                checkNumberOperand(expr->op,left,right);
                return std::any_cast<double>(left) >= std::any_cast<double>(right);
            }

            case(LESS):{
                checkNumberOperand(expr->op,left,right);
                return std::any_cast<double>(left) < std::any_cast<double>(right);
            }

            case(LESS_EQUAL):{
                checkNumberOperand(expr->op,left,right);
                return std::any_cast<double>(left) <= std::any_cast<double>(right);
            }

            case(EQUAL_EQUAL):{
                return isEqual(left,right);
            }
            case(BANG_EQUAL):{
                return !isEqual(left,right);
            }

            default: break;
        }
    
    return nullptr;
    
    }

    // Evaluate ternary operations
    std::any visitTernaryExpr(std::shared_ptr<Ternary> expr) override {
        // Evaluate left operand, if true : evaluate middle, else evalute right
        std::any left_eval = evaluate(expr->left);
        if(std::any_cast<bool>(left_eval)){
            return evaluate(expr->middle);
        }

        return evaluate(expr->right);
    }

private:

    std::shared_ptr<Environment> environment{new Environment};
    
    void execute(std::shared_ptr<Stmt> stmt){
        stmt->accept(*this);
    }

    // Execute a list of statements in the context of a given environment
    void executeBlock(std::vector<std::shared_ptr<Stmt>> statements, std::shared_ptr<Environment> environment){
        // Store the actual env. to restore the interpreter state
        // Bcs blocks will be executed in their own environment

        std::shared_ptr<Environment> previous = this->environment; 
        // Try and catch used here to restore the state even if the program fails
        // Throw the error after restoring
        try{
            // Use the newly created environment for the block
            this->environment = environment;

            for(const std::shared_ptr<Stmt>& statement : statements)
                execute(statement);

        } catch(...) {
            this->environment = previous;
            throw;
        }

        this->environment = previous;
    }

    std::any evaluate(std::shared_ptr<Expr> expr){
        return expr->accept(*this);
    }

    // false and null are considered to be Falsey, rest all truthy
    // eg. if(1) -> true ; if(null) -> false
    bool isTruthy(const std::any& obj){
        auto& value_type = obj.type();

        if(value_type == typeid(bool)) return std::any_cast<bool>(obj);
        if(value_type == typeid(nullptr)) return false;

        return true;
    }

    bool isEqual(std::any& left, std::any& right){
        if(left.type() == typeid(nullptr) && right.type() == typeid(nullptr)) return true;
        if(left.type() == typeid(nullptr)) return false;

        // check for string
        if(left.type() == typeid(std::string) && right.type() == typeid(std::string)) {
            return std::any_cast<std::string>(left) == std::any_cast<std::string>(right);
        }

        // check for double
        if(left.type() == typeid(double) && right.type() == typeid(double)) {
            return std::any_cast<double>(left) == std::any_cast<double>(right);
        }

        // check for bool
        if(left.type() == typeid(bool) && right.type() == typeid(bool)) {
            return std::any_cast<bool>(left) == std::any_cast<bool>(right);
        }

        return false;
    }

    void checkNumberOperand(Token opt, std::any operand){
        if(operand.type() == typeid(double)) return;

        throw RuntimeError(opt, "Operand must be a number.");
    }

    void checkNumberOperand(Token opt, std::any left, std::any right){
        if(left.type() == typeid(double) && right.type() == typeid(double)) return;

        throw RuntimeError(opt, "Operand must be a number.");
    }
    
    std::string stringify(const std::any& object) {
    if (object.type() == typeid(nullptr)) return std::string("nil");

    if (object.type() == typeid(double)) {
      std::string text = std::to_string(std::any_cast<double>(object));
      if (text[text.length() - 2] == '.' &&
          text[text.length() - 1] == '0') {
        text = text.substr(0, text.length() - 2);
      }
      return text;
    }

    if (object.type() == typeid(std::string)) {
      return std::any_cast<std::string>(object);
    }
    if (object.type() == typeid(bool)) {
      return std::any_cast<bool>(object) ? std::string("true") : std::string("false");
    }

    return "Error in stringify: object type not recognized.";
  }
};