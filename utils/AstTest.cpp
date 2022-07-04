#include<string>
#include <cstring>      // std::strerror
#include<fstream>
#include<iostream>
#include<vector>
#include"error.h"
#include"../scanner/scanner.h"
#include"../parser/parser.h"
#include"AstPrinter.h"

int main(int argc, char* argv[]) {
  std::shared_ptr<Expr> expression = std::make_shared<Binary>(
      std::make_shared<Unary>(
          Token{MINUS, "-", nullptr, 1},
          std::make_shared<Literal>(123.)
      ),
      Token{STAR, "*", nullptr, 1},
      std::make_shared<Grouping>(
          std::make_shared<Literal>(true)));
  
  std::shared_ptr<Expr> expression2 = std::make_shared<Binary>(
      std::make_shared<Binary>(
          std::make_shared<Literal>(1.),
          Token{MINUS, "+", nullptr, 1},
          std::make_shared<Literal>(2.)
      ),
      Token{STAR, "*", nullptr, 1},
      std::make_shared<Binary>(
          std::make_shared<Literal>(4.),
          Token{MINUS, "-", nullptr, 1},
          std::make_shared<Literal>(3.) )
          );

  std::cout << AstPrinter{}.print(expression) << "\n";
//   std::cout << AstPrinterRPN{}.print(expression2) << "\n";
}