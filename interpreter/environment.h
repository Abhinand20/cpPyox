#pragma once

#include<unordered_map>
#include<iostream>
#include<any>
#include<string>
#include"../utils/error.h"
#include"../scanner/token.h"

class Environment: public std::enable_shared_from_this<Environment> {

public:

    Environment() : enclosing(nullptr) {}
    
    Environment(std::shared_ptr<Environment> _enclosing) : enclosing(_enclosing) {}

    void define(std::string name, std::any value){
        values[name] = std::move(value);
    }
    
    std::any get(Token name){
        if(values.find(name.lexeme) != values.end()){
            return values[name.lexeme];
        }

        // If variable is not in current local scope, look for it in the outer scope
        if(enclosing != nullptr){
            return enclosing->get(name);
        }

        throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");

    }

    void assign(Token name, std::any value){
        if(values.find(name.lexeme) != values.end()){
            values[name.lexeme] = value;
            return;
        }
        
        // If variable is not in current local scope, look for it in the outer scope
        if(enclosing != nullptr){
            enclosing->assign(name,value);
            return;
        }

        throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
    }


private:
    std::unordered_map<std::string,std::any> values;
    // Reference to parent environment for each nested env.
    std::shared_ptr<Environment> enclosing;
};  

