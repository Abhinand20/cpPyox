#pragma once

#include<iostream>
#include<string>
#include<vector>
#include<utility>
#include<unordered_map>
#include"../utils/error.h"
#include"token.h"

class Scanner{

private:
    static const std::unordered_map<std::string,TokenType> keywords;
    const std::string source;
    std::vector<Token> tokens;
    int start = 0;
    int current = 0;
    int line = 1;

public:
    Scanner(std::string source) : source(source) {}

    // Main function to parse the file and store tokens
    std::vector<Token> scanTokens(){
        // Read chars and populate the tokens variable
        while(!isAtEnd()){
            start = current;
            scanToken();
        }      
        
        // Add an EOF token at the end of file
        Token* last = new Token(END_OF_FILE,"",nullptr,line);
        tokens.push_back(*last);
        return tokens;
    }

    // Helper : check end of file
    bool isAtEnd(){
        return current >= source.size();
    }
    
    // Helper : consume current character and move ahead
    char advance(){
        return source[current++];
    }
    
    // Helper : parse the current token
    void addToken(TokenType type){
        addToken(type,nullptr);
    }

    void addToken(TokenType type, std::any literal){
        std::string text = source.substr(start,current - start);
        tokens.push_back(Token(type, text,literal, line));
    }
    
    // Helper : Match next character of lexeme
    bool match(char expected){
        if(isAtEnd()) return false;
        if(source[current] != expected) return false;
        ++current;
        return true;
    }

    // Helper : Look ahead by 1 character
    char peek(){
        if(isAtEnd()) return '\0';
        return source[current];
    }
    
    // Helper : Look ahead by 2 characters
    char peekNext(){
        if(current + 1 >= source.size()) return '\0';

        return source[current+1];
    }

    // Process string literals
    void string(){
        // Keep consuming characters until we reach the end of string literal eg. "abc" or if we reach the end of file.
        while(peek() != '"' && !isAtEnd()){
            if(peek() == '\n'){
                ++line;
            }
            advance();
        }

        // If file has ended already and we did not encounter closing '"' - error
        if(isAtEnd()){
            error(line, "Unterminated string.");
        }

        advance(); // One more time to consume the closing '"'

        std::string value = source.substr(start+1,current - 2 - start);

        addToken(STRING,value);

    }

    // Process numeric literals
    void number(){
        // If we have digits remaining, continue consuming the characters as a single token
        while(isDigit(peek())) advance();

        // To confirm if its a decimal number, check the char after '.'
        if(peek() == '.' && isDigit(peekNext())){
            advance();

            while(isDigit(peek())) advance();
        }

        // Convert the string to "double"
        addToken(NUMBER,std::stod(source.substr(start,current - start)));
    }
    // Process identifiers and keywords
    void identifier(){
        while(isAlphaNumeric(peek())) {
            advance();
        }

        std::string text = source.substr(start,current-start);
        TokenType type;
        // Substring is a keyword
        if(keywords.find(text) != keywords.end()){
            // addToken(keywords[text]); // Cannot use [] in const map because if key is not found, it tries to add it in map
            type = keywords.at(text);
        }
        // Substring is an identifier
        else type = IDENTIFIER;
        
        if(type == TRUE) {
            addToken(type,true);
        }
        else if(type == FALSE){
            addToken(type,false);
        }
        else addToken(type);
        
    }
    
    // Helper : Check if char a digit
    bool isDigit(char c){
        return (c >= '0' && c <= '9');
    }
    // Helper : Check if char starts with alphabet
    bool isAlpha(char c){
        return ( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') );
    }
    // Helper : Check if alphanumeric
    bool isAlphaNumeric(char c){
        return ( isAlpha(c) || isDigit(c) ) ;
    }
    // Function to consume file characters
    //   1. Consider single character lexemes first
    //   2. Handle double character operators
    //   3. Handle special cases like comments, white spaces etc
    //   4. Handle longer lexemes like string/numeric literals
    //   5. Handle identifiers (variables) and keywords. (Use maximal munch algorithm)
    void scanToken(){
        char c = advance();
        switch(c){
            /// Single character lexemes ///
            case '(' : addToken(LEFT_PAREN); break;
            case ')' : addToken(RIGHT_PAREN); break;
            case '{' : addToken(LEFT_BRACE); break;
            case '}' : addToken(RIGHT_BRACE); break;
            case ',' : addToken(COMMA); break;
            case '.' : addToken(DOT); break;
            case '-' : addToken(MINUS); break;
            case '+' : addToken(PLUS); break;
            case ';' : addToken(SEMICOLON); break;
            case '*' : addToken(STAR); break;
            case '?' : addToken(QUESTION); break;
            case ':' : addToken(COLON); break;
            /// Special case - '/' can mean division or a comment ///
            case '/' : 
                // If its a comment : Just skip the following characters
                if(match('/')) {
                    while( peek() != '\n' && !isAtEnd()) {
                        advance();
                    }
                }
                // If multiline block comment  ( /* ... */ )
                else if(match('*')) {
                    while( !isAtEnd() && peek() != '*' && peekNext() != '/' ){
                        if(peek() == '\n') ++line;
                        advance();
                    }
                    if(isAtEnd()){
                        error(line, "Unterminated block comment.");
                    }

                    // To consume closing "*/"
                    advance(); 
                    advance();
                }
                else addToken(SLASH); 
                break;
            /// Skip over meaningless characters ///
            case ' ' :
            case '\r' :
            case '\t' : break; // Skip over white spaces
            case '\n' : line++; break;
            /// Double character lexemes ///
            case '=' : addToken(match('=') ? EQUAL_EQUAL : EQUAL); break;
            case '!' : addToken(match('=') ? BANG_EQUAL : BANG); break;
            case '>' : addToken(match('=') ? GREATER_EQUAL : GREATER); break;
            case '<' : addToken(match('=') ? LESS_EQUAL : LESS); break;
            /// Longer lexemes (literals) ///
            case '"' : string(); break; // String literals 
            default : 
                // Handle digits in default because its tedious to add a case for every decimal
                if(isDigit(c)){
                    number();
                }
                // Handle keywords and identifiers
                else if(isAlpha(c)){
                    identifier();
                }

                else error(line,"Unexpected character.");
                break;        

        }
    }

    
};

const std::unordered_map<std::string,TokenType> Scanner::keywords = {
    {"and",    AND},
    {"class",  CLASS},
    {"else",   ELSE},
    {"false",  FALSE},
    {"for",    FOR},
    {"fun",    FUN},
    {"if",     IF},
    {"nil",    NIL},
    {"or",     OR},
    {"print",  PRINT},
    {"return", RETURN},
    {"super",  SUPER},
    {"this",   THIS},
    {"true",   TRUE},
    {"var",    VAR},
    {"while",  WHILE}
};