#pragma once

#include<string>
#include<any>
#include"../utils/error.h"
#include"../utils/tokenType.h"
#include<utility>
/*
This class will process take in the parsed string and generate tokens
    - Methods to store and process tokens for each line of the input
*/
class Token{

public:
    const TokenType type;
    const std::string lexeme;
    // literal here has the actual value of the parsed token
    // It can be any type : NUMERIC, STRING etc so we store it as std::any type and process it later by checking type
    const std::any literal;
    const int line;

    // std::move - transfers resources from the given variable to another l-value
    // This reduces the overhead of creating copies if the variable being copied is not be used anymore
    Token(TokenType type, std::string lexeme, std::any literal, const int line) : type(type), lexeme(std::move(lexeme)), literal(literal), line(line) {};

    std::string toString(){
        
        std::string literal_text;

        switch (type) {
        case (IDENTIFIER):
            literal_text = lexeme;
            break;
        case (STRING):
            literal_text = std::any_cast<std::string>(literal);
            break;
        case (NUMBER):
            literal_text = std::to_string(std::any_cast<double>(literal));
            break;
        case (TRUE):
            literal_text = "true";
            break;
        case (FALSE):
            literal_text = "false";
            break;
        default:
            literal_text = "nil";
        }
        // Handle type of literal before returning
        return ::toString(type) + " " + lexeme + " " + literal_text;
    }
};