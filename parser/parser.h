#pragma once

#include<iostream>
#include<vector>
#include<string>
#include<stdexcept>
#include<utility> // std::move
#include"../interpreter/Stmt.h"
#include"../scanner/Expr.h"
#include"../utils/tokenType.h"
#include "../utils/error.h"

/*
This "recursive descent parser" (TOP-DOWN) follows the following grammar (precedence LOW->HIGH, top to bottom) -
    
    // Grammar for variable/functions/classes declarations
    #) program        → declaration* EOF ;
    #) declaration    → varDecl | statement ;
    #) varDecl        → "var" IDENTIFIER ( "=" comma )? ";" ;
    
    // General grammar for parsing statements
    *) statement      → exprStmt | printStmt ;
    *) exprStmt       → comma ";" ; (OR expression ";" ;)
    *) printStmt      → "print" expression ";" ;
    
    0) comma          → assignment ( (',') assignment )*
   +0) assignment     → IDENTIFIER "=" assignment | ternanry ;
   ++0) ternary       → ( expression "?" expression : )* expression
    1) expression     → equality ;
    2) equality       → comparison ( ( "!=" | "==" ) comparison )* ;
    3) comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
    4) term           → factor ( ( "-" | "+" ) factor )* ;
    5) factor         → unary ( ( "/" | "*" ) unary )* ;
    6) unary          → ( "!" | "-" ) unary | primary ;
    7) primary        → NUMBER | STRING | "true" 
                      | "false" | "nil" | "(" expression ")" 
                      | IDENTIFIER; 

Two main roles of a parser :
    1) Given a valid sequence of tokens, produce a corresponding syntax tree.
    2) Given an invalid sequence of tokens, detect any errors and tell the user about their mistakes.

Only supports single expression parsing for now

*** EXTENDED FUNCTIONALITY ***

a) Comma operator (,) : Lowest precedence

    0) comma → expression ( (',') expression )*


b) Ternary operator (?:) - right associative, precedence after comma
    +0) ternary → ( expression "?" expression : )* expression
******************


*/

class Parser{

public:
    
    Parser(std::vector<Token> _tokens) : tokens(_tokens) {};

    // Main function to kick off parsing
    // For now, if we face an error, we return null instead of sync (As we haven't implemented statements yet)
    std::vector<std::shared_ptr<Stmt>> parse(){
        try
        {
            std::vector<std::shared_ptr<Stmt>> statements;
            while(!isAtEnd()){
                statements.push_back(declaration());
            }

            return statements;
        }
        catch(ParseError error)
        {
            return {};
        }
        
    }


private:
    
    
    // (ERROR RECOVERY) This is a simple class to which we will use to catch the syntax errors
    // As we want to keep parsing the entire file
    struct ParseError: public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    const std::vector<Token> tokens;
    int current = 0;
    /// Helper functions ///
    
    template <class... T>
    bool match(T... types);
    
    bool check(TokenType type){
        if(isAtEnd()) return false;
        return peek().type == type;
    }
    
    // Helper : Consumes current token, returns it and advances
    Token advance(){
        if(!isAtEnd()) ++current;
        return previous();
    }
    bool isAtEnd(){
        return peek().type == END_OF_FILE;
    }

    Token peek(){
        return tokens[current];
    }

    Token previous(){
        return tokens[current-1];
    }

    //// ERROR RECOVERY ////

    // We return the error instead of throwing it because we want to let the calling method inside the parser decide whether to unwind or not.
    ParseError error(const Token& token, std::string message){
        ::error(token,message);
        return ParseError{""};
    }

    Token consume(TokenType type, std::string message){
        if(check(type)) return advance();

        throw error(peek(), message);
    }
    
    // This function is called when we catch a syntax error while parsing
    // Idea is to discard the current recursion stack and start over again
    // by ignoring all the remaining tokens until we reach a new statement
    // this prevents reporting cascading errors caused due to the very first error
    void synchronize(){
        advance();

        while(!isAtEnd()){
            if(previous().type == SEMICOLON) return;

            switch(peek().type){
                case CLASS:
                case FUN:
                case VAR:
                case FOR:
                case IF:
                case WHILE:
                case PRINT:
                case RETURN:
                    return;
                default:
                    break;
            }

            advance();
        }
    }
    
    
    /// Driver functions ///
    std::shared_ptr<Stmt> declaration();
    std::shared_ptr<Stmt> varDeclaration();
    std::shared_ptr<Stmt> statement();  
    std::shared_ptr<Stmt> printStatement();  
    std::shared_ptr<Stmt> expressionStatement();  
    std::shared_ptr<Expr> comma();      // 0th grammar rule
    std::shared_ptr<Expr> assignment();  
    std::shared_ptr<Expr> ternary();    // +0th grammar rule
    std::shared_ptr<Expr> expression(); // 1st grammar rule
    std::shared_ptr<Expr> equality();   // 2nd grammar rule
    std::shared_ptr<Expr> comparison(); // 3rd grammar rule
    std::shared_ptr<Expr> term();       // 4rd grammar rule
    std::shared_ptr<Expr> factor();     // 5rd grammar rule
    std::shared_ptr<Expr> unary();      // 6rd grammar rule
    std::shared_ptr<Expr> primary();    // 7rd grammar rule

};

// Matches current token with the provided types and consumes it
template <class... T>
bool Parser::match(T... types){

    for(auto& type : {types...}){
        if(check(type)){
            advance();
            return true;
        }
    }

    return false;

}

std::shared_ptr<Stmt> Parser::declaration(){
    try {
        if(match(VAR)) return varDeclaration();

        return statement();
    } catch (ParseError error){
        synchronize();
        return nullptr;
    }
}

std::shared_ptr<Stmt> Parser::varDeclaration(){
    Token name = consume(IDENTIFIER, "Expect variable name.");

    std::shared_ptr<Expr> initializer = nullptr;
    if(match(EQUAL)){
        // initializer = expression();
        initializer = comma();
    }

    consume(SEMICOLON, "Expect ';' after variable declaration.");
    
    return std::make_shared<Var>(name,initializer);
}

std::shared_ptr<Stmt> Parser::statement(){
    if(match(PRINT)) return printStatement();

    return expressionStatement();
}

std::shared_ptr<Stmt> Parser::expressionStatement(){
    std::shared_ptr<Expr> expr = comma();
    consume(SEMICOLON, "Exprect ';' after value.");

    return std::make_shared<Expression>(expr);
}
std::shared_ptr<Stmt> Parser::printStatement(){
    std::shared_ptr<Expr> value = expression();
    consume(SEMICOLON, "Expect ';' after value.");

    return std::make_shared<Print>(value);
}

std::shared_ptr<Expr> Parser::comma(){
    // std::shared_ptr<Expr> expr = expression();
    std::shared_ptr<Expr> expr = assignment();

    while(match(COMMA)) {
        
        Token op = previous();
        
        std::shared_ptr<Expr> right = assignment();

        expr = std::make_shared<Binary>(expr,std::move(op),right);
    }

    return expr;

}

std::shared_ptr<Expr> Parser::assignment(){
    std::shared_ptr<Expr> expr = expression(); // To get the left identifier

    if(match(EQUAL)){
        Token equals = previous();
        std::shared_ptr<Expr> value = assignment(); // Right-associative
        
        // Using dynamic_cast to check if Expr is of dervied type "Variable"
        // dynamic_cast<T*>(base) returns the pointer to T* after casting base to T if
        // T actually is of derived type. Otherwise return NULL (happens at runtime)
        if(Variable* v = dynamic_cast<Variable*>(expr.get())) {

            Token name = v->name;
            return std::make_shared<Assign>(std::move(name),value);
        }

        error(equals, "Invalid assignment target.");
    }

    return expr;
}

std::shared_ptr<Expr> Parser::ternary(){
    std::shared_ptr<Expr> expr = expression();

    // Are we in a conditional block?
    if(match(QUESTION)){
        // Store the "?"
        Token leftOp = previous();
        // Look for True
        std::shared_ptr<Expr> middle = ternary(); // Support nested statements like (a == b ? ( c == d ? d : e ) : f)
        // Look for the False
        if(match(COLON)){
            
            Token middleOp = previous();
            std::shared_ptr<Expr> right = ternary(); // Support nested statements like (a ? b : ( d == c ? e : f) : g )

            // Combine all these into a new AST node
            expr = std::make_shared<Ternary>(expr, leftOp, middle, middleOp, right);
        }
        // If we have a random "?" its an error
        else{
            throw error(peek(),"Expected ':' after ternary operator '?'.");
        }

    }

    return expr;

}
///// START : Binary operators (Lowest precedence) /////

// 1) & 2) -> equality -> comparision ( ("!=") | ("==") comparision )*
// Example : (sequence of operations) (== | !=) (sequence of operations)
std::shared_ptr<Expr> Parser::expression(){
    return equality();
}

// We keep parse the expressions on the left until we reach == or !=
// Then we parse the expressions on the right
// Combine both into a binary expr (eg. x == y)
std::shared_ptr<Expr> Parser::equality(){

    std::shared_ptr<Expr> expr = comparison();

    while(match(BANG_EQUAL, EQUAL_EQUAL)) {
        Token op = previous();
        std::shared_ptr<Expr> right = comparison();
        // Notice we are using expr as the left operator
        // For each iteration, we create a new binary expression using the previous one as the left operand.
        // eg. ( (a == b) == c )== d 
        expr = std::make_shared<Binary>(expr,std::move(op),right);
    }

    // If an equality operator is not found, we don't go inside the while loop
    // In this case we return comparision() as it is
    // This matches an equality operator or higher precedence
    return expr;
}


// 3) comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
// Exactly same logic as 1)
std::shared_ptr<Expr> Parser::comparison(){
    std::shared_ptr<Expr> expr = term();

    while(match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)){
        Token op = previous();
        std::shared_ptr<Expr> right = term();
        expr = std::make_shared<Binary>(expr,std::move(op),right);
    }

    return expr;
}

// 4) term → factor ( ( "-" | "+" ) factor )* ;
std::shared_ptr<Expr> Parser::term(){
    std::shared_ptr<Expr> expr = factor();

    while(match(MINUS, PLUS)){
        Token op = previous();
        std::shared_ptr<Expr> right = factor();
        expr = std::make_shared<Binary>(expr,std::move(op),right);
    }

    return expr;
}
///// END : Binary operators /////


///// START : Unary operators (Medium precedence) /////

// 5) factor → unary ( ( "/" | "*" ) unary )* ;
std::shared_ptr<Expr> Parser::factor(){
    std::shared_ptr<Expr> expr = unary();

    while(match(SLASH, STAR)){
        Token op = previous();
        std::shared_ptr<Expr> right = unary();
        expr = std::make_shared<Binary>(expr,std::move(op),right);
    }

    return expr;
}



// 6) unary → ( "!" | "-" ) unary | primary
std::shared_ptr<Expr> Parser::unary(){

    if(match(BANG, MINUS)){
        Token op = previous();
        // If we find "!" | "-" ; parse the expressions on the right recursively
        std::shared_ptr<Expr> right = unary();
        return std::make_shared<Unary>(std::move(op),right);
    }
    
    // Else, it must be a primary expression (Thats the only option left at this level of precedence)
    return primary();
}

///// END : Unary operators /////

//// START : Primary operators (Highest precedence) ////
// 7) primary → NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" ;
std::shared_ptr<Expr> Parser::primary(){
    if(match(FALSE)) return std::make_shared<Literal>(false);
    if(match(TRUE)) return std::make_shared<Literal>(true);
    if(match(NIL)) return std::make_shared<Literal>(nullptr);
    if(match(IDENTIFIER)) return std::make_shared<Variable>(previous());
    if(match(NUMBER,STRING)){
        return std::make_shared<Literal>(previous().literal);
    }
    // If we match a "(", we must find a ")" otherwise its an error
    if(match(LEFT_PAREN)){
        // Start recursively consuming expressions

        std::shared_ptr<Expr> expr = expression();
        // After parsing expression, next token must be ")"
        consume(RIGHT_PAREN, "Expect ')' after expression.");
        
        return std::make_shared<Grouping>(expr);

    }

    // If no cases match till now, we are at a token that is not a part of any expression
        

    throw error(peek(), "Expect expression.");

}