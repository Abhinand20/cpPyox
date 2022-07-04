#pragma once

#include <stdexcept>
#include "../scanner/token.h"

class RuntimeError : public std::runtime_error {
public:
    const Token& token;

    RuntimeError(const Token& _token, std::string msg) 
    : std::runtime_error{msg} , token{_token} 
    {}
};