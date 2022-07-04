#pragma once

#include <any>
#include <memory>
#include <utility>  // std::move
#include <vector>
#include "../scanner/token.h"
#include "../scanner/Expr.h"

struct Expression;
struct Print;
struct Var;

struct StmtVisitor {
virtual std::any visitExpressionStmt(std::shared_ptr<Expression> stmt) = 0;
virtual std::any visitPrintStmt(std::shared_ptr<Print> stmt) = 0;
virtual std::any visitVarStmt(std::shared_ptr<Var> stmt) = 0;
virtual ~StmtVisitor() = default;
};

struct Stmt
{
 virtual std::any accept(StmtVisitor& visitor) = 0;
};

struct Expression: Stmt, public std::enable_shared_from_this<Expression> {
  Expression(std::shared_ptr<Expr> expression)
  : expression{std::move(expression)}
  {}

  std::any accept(StmtVisitor& visitor) override {
    return visitor.visitExpressionStmt(shared_from_this());
  }

  const std::shared_ptr<Expr> expression;
};

struct Print: Stmt, public std::enable_shared_from_this<Print> {
  Print(std::shared_ptr<Expr> expression)
  : expression{std::move(expression)}
  {}

  std::any accept(StmtVisitor& visitor) override {
    return visitor.visitPrintStmt(shared_from_this());
  }

  const std::shared_ptr<Expr> expression;
};

struct Var: Stmt, public std::enable_shared_from_this<Var> {
  Var(Token name, std::shared_ptr<Expr> initializer)
  : name{std::move(name)}, initializer{std::move(initializer)}
  {}

  std::any accept(StmtVisitor& visitor) override {
    return visitor.visitVarStmt(shared_from_this());
  }

  const Token name;
  const std::shared_ptr<Expr> initializer;
};

