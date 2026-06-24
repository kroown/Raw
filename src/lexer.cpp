#include "lexer.hpp"
#include <cctype>
#include <unordered_map>

Lexer::Lexer(std::string source) : source(std::move(source)) {}

bool Lexer::is_at_end() const { return current >= source.size(); }

char Lexer::advance() {
  char c = source[current++];
  if (c == '\n') { line++; col = 1; }
  else { col++; }
  return c;
}

char Lexer::peek() const {
  if (is_at_end()) return '\0';
  return source[current];
}

char Lexer::peek_next() const {
  if (current + 1 >= source.size()) return '\0';
  return source[current + 1];
}

bool Lexer::match(char expected) {
  if (is_at_end() || source[current] != expected) return false;
  current++; col++;
  return true;
}

void Lexer::add_token(TokenType type) {
  tokens.push_back({type, source.substr(start, current - start), line, col});
}

void Lexer::add_token(TokenType type, std::string lexeme) {
  tokens.push_back({type, std::move(lexeme), line, col});
}

void Lexer::skip_whitespace() {
  while (!is_at_end()) {
    char c = peek();
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n') advance();
    else break;
  }
}

void Lexer::comment() {
  while (!is_at_end() && peek() != '\n') advance();
}

void Lexer::block_comment() {
  while (!is_at_end()) {
    if (peek() == '*' && peek_next() == '/') { advance(); advance(); return; }
    advance();
  }
}

void Lexer::string() {
  std::string val;
  while (!is_at_end() && peek() != '"') {
    if (peek() == '\\') {
      advance();
      switch (advance()) {
        case 'n': val += '\n'; break;
        case 't': val += '\t'; break;
        case '0': val += '\0'; break;
        case '"': val += '"'; break;
        case '\\': val += '\\'; break;
        default: val += '\\'; break;
      }
    } else {
      val += advance();
    }
  }
  if (is_at_end()) return;
  advance();
  tokens.push_back({TokenType::STRING, val, line, col});
}

void Lexer::number() {
  if (peek() == 'x' || peek() == 'X') {
    advance();
    while (std::isxdigit(peek())) advance();
  } else {
    while (std::isdigit(peek())) advance();
  }
  add_token(TokenType::INTEGER);
}

void Lexer::identifier() {
  while (std::isalnum(peek()) || peek() == '_') advance();
  std::string word = source.substr(start, current - start);

  static const std::unordered_map<std::string_view, TokenType> keywords = {
    {"fn", TokenType::FN}, {"return", TokenType::RETURN},
    {"if", TokenType::IF}, {"else", TokenType::ELSE},
    {"while", TokenType::WHILE}, {"for", TokenType::FOR},
    {"break", TokenType::BREAK}, {"continue", TokenType::CONTINUE},
    {"let", TokenType::LET}, {"extern", TokenType::EXTERN}, {"sizeof", TokenType::SIZEOF},
    {"int", TokenType::INT_KW}, {"char", TokenType::CHAR_KW},
    {"bool", TokenType::BOOL_KW}, {"str", TokenType::STR_KW},
    {"void", TokenType::VOID_KW},
    {"true", TokenType::TRUE_KW}, {"false", TokenType::FALSE_KW},
  };

  auto it = keywords.find(word);
  add_token(it != keywords.end() ? it->second : TokenType::IDENTIFIER);
}

std::vector<Token> Lexer::tokenize() {
  while (!is_at_end()) {
    skip_whitespace();
    start = current;
    if (is_at_end()) break;

    char c = advance();
    switch (c) {
      case '(': add_token(TokenType::LPAREN); break;
      case ')': add_token(TokenType::RPAREN); break;
      case '{': add_token(TokenType::LBRACE); break;
      case '}': add_token(TokenType::RBRACE); break;
      case '[': add_token(TokenType::LBRACKET); break;
      case ']': add_token(TokenType::RBRACKET); break;
      case ';': add_token(TokenType::SEMICOLON); break;
      case ',': add_token(TokenType::COMMA); break;
      case '.': add_token(TokenType::DOT); break;
      case ':':
        if (match(':')) {}
        else add_token(TokenType::COLON);
        break;
      case '+':
        if (match('+')) add_token(TokenType::PLUS_PLUS);
        else if (match('=')) add_token(TokenType::PLUS_EQUAL);
        else add_token(TokenType::PLUS);
        break;
      case '-':
        if (match('-')) add_token(TokenType::MINUS_MINUS);
        else if (match('=')) add_token(TokenType::MINUS_EQUAL);
        else if (match('>')) add_token(TokenType::ARROW);
        else add_token(TokenType::MINUS);
        break;
      case '*':
        if (match('=')) add_token(TokenType::STAR_EQUAL);
        else add_token(TokenType::STAR);
        break;
      case '/':
        if (match('/')) comment();
        else if (match('*')) block_comment();
        else if (match('=')) add_token(TokenType::SLASH_EQUAL);
        else add_token(TokenType::SLASH);
        break;
      case '%': add_token(TokenType::PERCENT); break;
      case '=':
        add_token(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
        break;
      case '!':
        add_token(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
        break;
      case '<':
        add_token(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
        break;
      case '>':
        add_token(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
        break;
      case '&': add_token(match('&') ? TokenType::PIPE : TokenType::AMPERSAND); break;
      case '|':
        if (match('|')) {}
        else add_token(TokenType::PIPE);
        break;
      case '~': add_token(TokenType::TILDE); break;
      case '"': string(); break;
      default:
        if (std::isdigit(c)) number();
        else if (std::isalpha(c) || c == '_') identifier();
        else add_token(TokenType::UNKNOWN);
        break;
    }
  }
  tokens.push_back({TokenType::END, "", line, col});
  return tokens;
}
