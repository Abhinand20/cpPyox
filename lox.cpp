#include<string>
#include <cstring>      // std::strerror
#include<fstream>
#include<iostream>
#include<vector>
#include"utils/error.h"
#include"scanner/scanner.h"
#include"parser/parser.h"
#include"utils/AstPrinter.h"
#include"interpreter/interpreter.h"
#include"interpreter/Stmt.h"

std::string readFile(std::string path) {
  std::ifstream file{path, std::ios::in | std::ios::binary |
                                  std::ios::ate};
  if (!file) {
    std::cerr << "Failed to open file " << path << ": "
              << std::strerror(errno) << "\n";
    std::exit(74);
  };

  std::string contents;
  contents.resize(file.tellg());

  file.seekg(0, std::ios::beg);
  file.read(contents.data(), contents.size());

  return contents;
}

void run(std::string source){
    Scanner scanObj(source);
    std::vector<Token> res;
    res = scanObj.scanTokens();

    Parser p(res);
    // // AstPrinter pprint;
    std::vector<std::shared_ptr<Stmt>> statements = p.parse();
    // // std::cout<<pprint.print(expr);
    // // std::cout<<std::endl;
    Interpreter eval;
    eval.interpret(statements);
}


void runFile(std::string path){
    std::string content = readFile(path);
    run(content);

    if(hadError) {
        std::exit(65);
    }
    if(hadRuntimeError){
        std::exit(70);
    }
}


void runPrompt(){
    std::string source;
    while(true){
        std::cout<<"> ";
        std::getline(std::cin,source);
        run(source);
        std::cout<<std::endl;
        hadError = false;
    }
}

int main(int argc, char** argv){
    if(argc > 2){
        std::cout<<"Usage: jlox [script]\n";
        std::exit(64);
    }
    else if(argc == 2){
        runFile(argv[1]);
    }
    else{
        std::cout<<"Interactive mode!"<<std::endl;
        runPrompt();
    }

    return 0;
}
