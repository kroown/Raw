#pragma once
#include "token.hpp"
#include <string>
#include <vector>

class Lexer {
public:
  explicit Lexer(std::string source);
  std::vector<Token> tokenize();

private:
  std::string source;
  size_t start = 0;
  size_t current = 0;
  size_t line = 1;
  size_t col = 1;
  std::vector<Token> tokens;

  void add_token(TokenType type);
  void add_token(TokenType type, std::string lexeme);
  char advance();
  bool match(char expected);
  void skip_whitespace();
  void comment();
  void block_comment();
  void string();
  void number();
  void identifier();
  char peek() const;
  char peek_next() const;
  bool is_at_end() const;
};
