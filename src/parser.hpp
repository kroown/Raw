#pragma once
#include "ast.hpp"
#include "token.hpp"
#include <memory>
#include <vector>

class Parser {
public:
  explicit Parser(std::vector<Token> tokens);
  std::unique_ptr<Program> parse();
  std::string error_msg() const { return error; }
  bool had_error() const { return errored; }

private:
  std::vector<Token> tokens;
  size_t pos = 0;
  bool errored = false;
  std::string error;

  const Token& peek() const;
  const Token& previous() const;
  const Token& advance();
  bool check(TokenType type) const;
  bool match(std::initializer_list<TokenType> types);
  bool match(TokenType type);
  Token consume(TokenType type, std::string msg);
  void parse_error(std::string msg);

  std::unique_ptr<FnDecl> function();
  void extern_block(Program& prog);
  std::unique_ptr<Stmt> statement();
  std::unique_ptr<Stmt> var_decl();
  std::unique_ptr<Stmt> if_stmt();
  std::unique_ptr<Stmt> while_stmt();
  std::unique_ptr<Stmt> for_stmt();
  std::unique_ptr<Stmt> return_stmt();
  std::unique_ptr<Stmt> break_stmt();
  std::unique_ptr<Stmt> continue_stmt();
  std::unique_ptr<Stmt> expression_stmt();
  std::unique_ptr<Block> block();
  std::vector<Param> params();
  Type parse_type();

  std::unique_ptr<Expr> expression();
  std::unique_ptr<Expr> assignment();
  std::unique_ptr<Expr> logical_or();
  std::unique_ptr<Expr> logical_and();
  std::unique_ptr<Expr> equality();
  std::unique_ptr<Expr> comparison();
  std::unique_ptr<Expr> term();
  std::unique_ptr<Expr> factor();
  std::unique_ptr<Expr> unary();
  std::unique_ptr<Expr> postfix();
  std::unique_ptr<Expr> primary();
};
