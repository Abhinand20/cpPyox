#include<iostream>
#include<sstream>
#include<string>
#include"../scanner/Expr.h"
#include"../scanner/token.h"
#include<type_traits>
#include<any>

class AstPrinter : public ExprVisitor {
private:
    // Need a multi-parameter template class - we might pass std::shared_ptr<Literal>, Unary etc any of those
    template <class... E> // Expect multiple template  params (class... E)
    std::string parenthesize(std::string name, E... expr ){ // Expand the arguments E...
        
        std::ostringstream builder;

        builder<< "(" << name;
        for( auto& e : {expr...}){
            builder<<" ";
            builder<<print(e);
        }
        builder << ")";

        return builder.str();
    }

public:
    std::string print(std::shared_ptr<Expr> expr){
        // return std::any_cast<std::string>(expr->accept(*this)); // *this passes a copy of current object
        return std::any_cast<std::string>(expr->accept(*this));

    }

    std::any visitTernaryExpr(std::shared_ptr<Ternary> expr) override {
        return parenthesize(expr->leftOp.lexeme + " " + expr->middleOp.lexeme, expr->left, expr->middle, expr->right);
    }

    std::any visitBinaryExpr(std::shared_ptr<Binary> expr) override {
        return parenthesize(expr->op.lexeme, expr->left, expr->right);
    }

    std::any visitUnaryExpr(std::shared_ptr<Unary> expr) override {
        return parenthesize(expr->op.lexeme,expr->right);
    }

    std::any visitGroupingExpr(std::shared_ptr<Grouping> expr) override {
        return parenthesize("group", expr->expression);
    }

    std::any visitLiteralExpr(std::shared_ptr<Literal> expr) override {
        // We don't know the type of literal so we handle it before converting to string
        auto& value_type = expr->value.type();
        
        if(value_type == typeid(nullptr)){
            return std::string("nil");
        }
        else if(value_type == typeid(std::string)){
            return std::any_cast<std::string>(expr->value);
        }
        else if(value_type == typeid(bool)){
            return std::any_cast<bool>(expr->value) ? std::string("true") : std::string("false");
        }
        else if(value_type == typeid(double)){
            return std::to_string(std::any_cast<double>(expr->value));
        }

        return "Error in visitLiteralExpr: literal type not recognized.";
    }

};

class AstPrinterRPN : public ExprVisitor {
private:
    // Need a multi-parameter template class - we might pass std::shared_ptr<Literal>, Unary etc any of those
    template <class... E> // Expect multiple template  params (class... E)
    std::string parenthesize(std::string name, E... expr ){ // Expand the arguments E...
        
        std::ostringstream builder;

        builder<< "(";
        for( auto& e : {expr...}){
            
            builder<<print(e);

            builder<<" ";
        }
        builder << name << ")";

        return builder.str();
    }

public:
    std::string print(std::shared_ptr<Expr> expr){
        return std::any_cast<std::string>(expr->accept(*this)); // *this passes a copy of current object
    }

    std::any visitBinaryExpr(std::shared_ptr<Binary> expr) override {
        return parenthesize(expr->op.lexeme, expr->left, expr->right);
    }

    std::any visitUnaryExpr(std::shared_ptr<Unary> expr) override {
        return parenthesize(expr->op.lexeme,expr->right);
    }

    std::any visitGroupingExpr(std::shared_ptr<Grouping> expr) override {
        return parenthesize("group", expr->expression);
    }

    

    std::any visitLiteralExpr(std::shared_ptr<Literal> expr) override {
        // We don't know the type of literal so we handle it before converting to string
        auto& value_type = expr->value.type();

        if(value_type == typeid(nullptr)){
            return std::string("nil");
        }
        if(value_type == typeid(std::string)){
            return std::any_cast<std::string>(expr->value);
        }
        if(value_type == typeid(bool)){
            return std::any_cast<bool>(expr->value) ? std::string("true") : std::string("false");
        }
        if(value_type == typeid(double)){
            return std::to_string(std::any_cast<double>(expr->value));
        }

        return "Error in visitLiteralExpr: literal type not recognized.";
    }

};
