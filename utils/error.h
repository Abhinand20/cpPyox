#pragma once

#include<iostream>
#include<string>
#include"../scanner/token.h"
#include"runtimeError.h"
// Using inline here will declare hadError to be a global variable which chan be shared among different compilation units
// Not using static - A static function in a header will get compiled into every source file which includes it - so there will be lots of copies of it
inline bool hadError = false; 
inline bool hadRuntimeError = false; 

static void report(int line, std::string where, std::string message){
    std::cerr<<"[line : "<<line<<"] Error - "<<message<<std::endl;
    hadError = true;
}

static void error(const Token& token, std::string message){
    if(token.type == END_OF_FILE) {
        report(token.line, " at end", message);
    }
    else{
        report(token.line, " at '" + token.lexeme + "'",message);
    }
}

static void error(int line, std::string message){
    report(line,"",message);
}

static void runtimeError(const RuntimeError& error){
    std::cerr<<std::string(error.what()) + "\n[line " << error.token.line << "]";

    hadRuntimeError = true;

}