#pragma once

#include<unordered_map>
#include<iostream>
#include<any>
#include<string>
#include"../utils/error.h"
#include"../scanner/token.h"

class Environment: public std::enable_shared_from_this<Environment> {

public:
    void define(std::string name, std::any value){
        values[name] = std::move(value);
    }
    
    std::any get(Token name){
        if(values.find(name.lexeme) != values.end()){
            return values[name.lexeme];
        }

        throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");

    }

    void assign(Token name, std::any value){
        if(values.find(name.lexeme) != values.end()){
            values[name.lexeme] = value;
            return;
        }

        throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
    }


private:
    std::unordered_map<std::string,std::any> values;
};

