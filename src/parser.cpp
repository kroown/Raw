#include "parser.hpp"

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

const Token& Parser::peek() const { return tokens[pos]; }
const Token& Parser::previous() const { return tokens[pos - 1]; }
const Token& Parser::advance() {
  if (!check(TokenType::END)) pos++;
  return previous();
}
bool Parser::check(TokenType type) const { return peek().type == type; }
bool Parser::match(std::initializer_list<TokenType> types) {
  for (auto t : types) { if (check(t)) { advance(); return true; } }
  return false;
}
bool Parser::match(TokenType type) {
  if (check(type)) { advance(); return true; }
  return false;
}
Token Parser::consume(TokenType type, std::string msg) {
  if (check(type)) return advance();
  parse_error(msg + " at '" + peek().lexeme + "'");
  return {};
}
void Parser::parse_error(std::string msg) {
  if (errored) return;
  errored = true;
  error = msg;
}

Type Parser::parse_type() {
  Type t;
  if (match(TokenType::INT_KW)) t.kind = Type::INT;
  else if (match(TokenType::CHAR_KW)) t.kind = Type::CHAR;
  else if (match(TokenType::BOOL_KW)) t.kind = Type::BOOL;
  else if (match(TokenType::STR_KW)) t.kind = Type::STR;
  else if (match(TokenType::VOID_KW)) t.kind = Type::VOID;
  else { parse_error("expected type"); t.kind = Type::INT; return t; }

  while (match(TokenType::STAR)) t.ptr_depth++;
  if (match(TokenType::LBRACKET)) {
    Token size_tok = consume(TokenType::INTEGER, "expected array size");
    t.array_size = std::stoi(size_tok.lexeme, nullptr, 0);
    consume(TokenType::RBRACKET, "expected ']'");
  }
  return t;
}

std::unique_ptr<Program> Parser::parse() {
  auto prog = std::make_unique<Program>();
  while (!check(TokenType::END)) {
    if (check(TokenType::FN)) prog->functions.push_back(function());
    else if (check(TokenType::EXTERN)) extern_block(*prog);
    else { parse_error("expected function"); break; }
  }
  return prog;
}

std::unique_ptr<FnDecl> Parser::function() {
  consume(TokenType::FN, "expected 'fn'");
  Token name = consume(TokenType::IDENTIFIER, "expected function name");
  consume(TokenType::LPAREN, "expected '('");
  auto p = params();
  consume(TokenType::RPAREN, "expected ')'");
  Type ret{Type::VOID};
  if (match(TokenType::ARROW)) ret = parse_type();
  auto body = block();
  if (!body) body = std::make_unique<Block>();
  auto fn = std::make_unique<FnDecl>();
  fn->name = name.lexeme;
  fn->params = std::move(p);
  fn->return_type = ret;
  fn->body = std::move(body);
  return fn;
}

void Parser::extern_block(Program& prog) {
  consume(TokenType::EXTERN, "expected 'extern'");
  consume(TokenType::LBRACE, "expected '{'");
  while (!check(TokenType::RBRACE) && !check(TokenType::END)) {
    ExternFnDecl efn;
    consume(TokenType::FN, "expected 'fn'");
    Token name = consume(TokenType::IDENTIFIER, "expected function name");
    consume(TokenType::LPAREN, "expected '('");
    if (!check(TokenType::RPAREN)) {
      do {
        Param p;
        Token pname = consume(TokenType::IDENTIFIER, "expected parameter name");
        consume(TokenType::COLON, "expected ':'");
        p.name = pname.lexeme;
        p.type = parse_type();
        if (!errored) efn.params.push_back(p);
      } while (match(TokenType::COMMA) && !errored);
    }
    consume(TokenType::RPAREN, "expected ')'");
    efn.return_type = Type{Type::VOID};
    if (match(TokenType::ARROW)) efn.return_type = parse_type();
    efn.name = name.lexeme;
    consume(TokenType::SEMICOLON, "expected ';'");
    if (!errored) prog.extern_functions.push_back(std::move(efn));
  }
  consume(TokenType::RBRACE, "expected '}'");
}

std::vector<Param> Parser::params() {
  std::vector<Param> p;
  if (check(TokenType::RPAREN)) return p;
  do {
    Token name = consume(TokenType::IDENTIFIER, "expected parameter name");
    consume(TokenType::COLON, "expected ':'");
    Type type = parse_type();
    if (!errored) p.push_back({name.lexeme, type});
  } while (match(TokenType::COMMA) && !errored);
  return p;
}

std::unique_ptr<Block> Parser::block() {
  if (!match(TokenType::LBRACE)) { parse_error("expected '{'"); return nullptr; }
  auto b = std::make_unique<Block>();
  while (!check(TokenType::RBRACE) && !check(TokenType::END)) {
    auto stmt = statement();
    if (stmt) b->statements.push_back(std::move(stmt));
    else break;
  }
  consume(TokenType::RBRACE, "expected '}'");
  return b;
}

std::unique_ptr<Stmt> Parser::statement() {
  if (match(TokenType::LET)) return var_decl();
  if (match(TokenType::IF)) return if_stmt();
  if (match(TokenType::WHILE)) return while_stmt();
  if (match(TokenType::FOR)) return for_stmt();
  if (match(TokenType::RETURN)) return return_stmt();
  if (match(TokenType::BREAK)) return break_stmt();
  if (match(TokenType::CONTINUE)) return continue_stmt();
  if (match(TokenType::LBRACE)) { pos--; return block(); }
  return expression_stmt();
}

std::unique_ptr<Stmt> Parser::var_decl() {
  Token name = consume(TokenType::IDENTIFIER, "expected variable name");
  consume(TokenType::COLON, "expected ':'");
  Type type = parse_type();
  std::unique_ptr<Expr> init;
  if (match(TokenType::EQUAL)) init = expression();
  consume(TokenType::SEMICOLON, "expected ';'");
  auto v = std::make_unique<VarDeclStmt>();
  v->name = name.lexeme;
  v->type = type;
  v->initializer = std::move(init);
  return v;
}

std::unique_ptr<Stmt> Parser::if_stmt() {
  auto cond = expression();
  auto then_branch = block();
  std::unique_ptr<Stmt> else_branch;
  if (match(TokenType::ELSE)) {
    if (match(TokenType::IF)) { pos--; else_branch = if_stmt(); }
    else else_branch = block();
  }
  auto i = std::make_unique<IfStmt>();
  i->condition = std::move(cond);
  i->then_branch = std::move(then_branch);
  i->else_branch = std::move(else_branch);
  return i;
}

std::unique_ptr<Stmt> Parser::while_stmt() {
  auto cond = expression();
  auto body = block();
  auto w = std::make_unique<WhileStmt>();
  w->condition = std::move(cond);
  w->body = std::move(body);
  return w;
}

std::unique_ptr<Stmt> Parser::for_stmt() {
  std::unique_ptr<Stmt> init;
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Expr> inc;
  if (match(TokenType::LET)) {
    Token name = consume(TokenType::IDENTIFIER, "expected variable name");
    consume(TokenType::COLON, "expected ':'");
    Type type = parse_type();
    std::unique_ptr<Expr> val;
    if (match(TokenType::EQUAL)) val = expression();
    auto v = std::make_unique<VarDeclStmt>();
    v->name = name.lexeme; v->type = type; v->initializer = std::move(val);
    init = std::move(v);
    consume(TokenType::SEMICOLON, "expected ';'");
  } else {
    consume(TokenType::SEMICOLON, "expected ';'");
  }
  if (!check(TokenType::SEMICOLON)) cond = expression();
  consume(TokenType::SEMICOLON, "expected ';'");
  if (!check(TokenType::LBRACE)) inc = expression();
  auto body = block();
  auto f = std::make_unique<ForStmt>();
  f->init = std::move(init);
  f->condition = std::move(cond);
  f->inc = std::move(inc);
  f->body = std::move(body);
  return f;
}

std::unique_ptr<Stmt> Parser::return_stmt() {
  std::unique_ptr<Expr> val;
  if (!check(TokenType::SEMICOLON)) val = expression();
  consume(TokenType::SEMICOLON, "expected ';'");
  auto r = std::make_unique<ReturnStmt>();
  r->value = std::move(val);
  return r;
}

std::unique_ptr<Stmt> Parser::break_stmt() {
  consume(TokenType::SEMICOLON, "expected ';'");
  return std::make_unique<BreakStmt>();
}

std::unique_ptr<Stmt> Parser::continue_stmt() {
  consume(TokenType::SEMICOLON, "expected ';'");
  return std::make_unique<ContinueStmt>();
}

std::unique_ptr<Stmt> Parser::expression_stmt() {
  auto e = expression();
  consume(TokenType::SEMICOLON, "expected ';'");
  auto es = std::make_unique<ExprStmt>();
  es->expr = std::move(e);
  return es;
}

std::unique_ptr<Expr> Parser::expression() {
  return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
  auto expr = equality();
  if (match({TokenType::EQUAL, TokenType::PLUS_EQUAL, TokenType::MINUS_EQUAL,
             TokenType::STAR_EQUAL, TokenType::SLASH_EQUAL})) {
    TokenType op_t = previous().type;
    auto val = assignment();
    auto a = std::make_unique<BinaryExpr>();
    a->left = std::move(expr);
    switch (op_t) {
      case TokenType::EQUAL: a->op = BinaryExpr::ASSIGN; break;
      case TokenType::PLUS_EQUAL: a->op = BinaryExpr::PLUS_ASSIGN; break;
      case TokenType::MINUS_EQUAL: a->op = BinaryExpr::MINUS_ASSIGN; break;
      case TokenType::STAR_EQUAL: a->op = BinaryExpr::STAR_ASSIGN; break;
      case TokenType::SLASH_EQUAL: a->op = BinaryExpr::SLASH_ASSIGN; break;
      default: break;
    }
    a->right = std::move(val);
    return a;
  }
  return expr;
}

std::unique_ptr<Expr> Parser::equality() {
  auto expr = comparison();
  while (match({TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL})) {
    Token op = previous();
    auto right = comparison();
    auto b = std::make_unique<BinaryExpr>();
    b->left = std::move(expr);
    b->op = op.type == TokenType::EQUAL_EQUAL ? BinaryExpr::EQUAL_EQUAL : BinaryExpr::BANG_EQUAL;
    b->right = std::move(right);
    expr = std::move(b);
  }
  return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
  auto expr = term();
  while (match({TokenType::LESS, TokenType::LESS_EQUAL, TokenType::GREATER, TokenType::GREATER_EQUAL})) {
    Token op = previous();
    auto right = term();
    auto b = std::make_unique<BinaryExpr>();
    b->left = std::move(expr);
    switch (op.type) {
      case TokenType::LESS: b->op = BinaryExpr::LESS; break;
      case TokenType::LESS_EQUAL: b->op = BinaryExpr::LESS_EQUAL; break;
      case TokenType::GREATER: b->op = BinaryExpr::GREATER; break;
      case TokenType::GREATER_EQUAL: b->op = BinaryExpr::GREATER_EQUAL; break;
      default: break;
    }
    b->right = std::move(right);
    expr = std::move(b);
  }
  return expr;
}

std::unique_ptr<Expr> Parser::term() {
  auto expr = factor();
  while (match({TokenType::PLUS, TokenType::MINUS})) {
    Token op = previous();
    auto right = factor();
    auto b = std::make_unique<BinaryExpr>();
    b->left = std::move(expr);
    b->op = op.type == TokenType::PLUS ? BinaryExpr::PLUS : BinaryExpr::MINUS;
    b->right = std::move(right);
    expr = std::move(b);
  }
  return expr;
}

std::unique_ptr<Expr> Parser::factor() {
  auto expr = unary();
  while (match({TokenType::STAR, TokenType::SLASH})) {
    Token op = previous();
    auto right = unary();
    auto b = std::make_unique<BinaryExpr>();
    b->left = std::move(expr);
    b->op = op.type == TokenType::STAR ? BinaryExpr::STAR : BinaryExpr::SLASH;
    b->right = std::move(right);
    expr = std::move(b);
  }
  return expr;
}

std::unique_ptr<Expr> Parser::unary() {
  if (match({TokenType::BANG, TokenType::MINUS, TokenType::AMPERSAND, TokenType::STAR,
             TokenType::PLUS_PLUS, TokenType::MINUS_MINUS})) {
    TokenType op_t = previous().type;
    auto operand = unary();
    auto u = std::make_unique<UnaryExpr>();
    switch (op_t) {
      case TokenType::BANG: u->op = UnaryExpr::NOT; break;
      case TokenType::MINUS: u->op = UnaryExpr::NEG; break;
      case TokenType::AMPERSAND: u->op = UnaryExpr::ADDR; break;
      case TokenType::STAR: u->op = UnaryExpr::DEREF; break;
      case TokenType::PLUS_PLUS: u->op = UnaryExpr::PLUS_PLUS; break;
      case TokenType::MINUS_MINUS: u->op = UnaryExpr::MINUS_MINUS; break;
      default: break;
    }
    u->operand = std::move(operand);
    return u;
  }
  return postfix();
}

std::unique_ptr<Expr> Parser::postfix() {
  auto expr = primary();
  while (true) {
    if (match(TokenType::LPAREN)) {
      auto c = std::make_unique<CallExpr>();
      if (auto* v = dynamic_cast<VariableExpr*>(expr.get())) {
        c->callee = v->name;
      } else {
        parse_error("invalid callee"); return expr;
      }
      while (!check(TokenType::RPAREN) && !check(TokenType::END)) {
        c->args.push_back(expression());
        if (!check(TokenType::RPAREN)) consume(TokenType::COMMA, "expected ',' or ')'");
      }
      consume(TokenType::RPAREN, "expected ')'");
      expr = std::move(c);
    } else if (match(TokenType::LBRACKET)) {
      auto idx = expression();
      consume(TokenType::RBRACKET, "expected ']'");
      auto i = std::make_unique<IndexExpr>();
      i->base = std::move(expr);
      i->index = std::move(idx);
      expr = std::move(i);
    } else if (match(TokenType::DOT)) {
      Token member = consume(TokenType::IDENTIFIER, "expected member name");
      auto m = std::make_unique<MemberExpr>();
      m->base = std::move(expr);
      m->member = member.lexeme;
      expr = std::move(m);
    } else break;
  }
  return expr;
}

std::unique_ptr<Expr> Parser::primary() {
  if (match(TokenType::INTEGER)) {
    auto i = std::make_unique<IntegerExpr>();
    i->value = std::stoll(previous().lexeme, nullptr, 0);
    return i;
  }
  if (match(TokenType::STRING)) {
    auto s = std::make_unique<StringExpr>();
    s->value = previous().lexeme;
    return s;
  }
  if (match(TokenType::TRUE_KW)) { auto b = std::make_unique<BoolExpr>(); b->value = true; return b; }
  if (match(TokenType::FALSE_KW)) { auto b = std::make_unique<BoolExpr>(); b->value = false; return b; }
  if (match(TokenType::IDENTIFIER)) {
    auto v = std::make_unique<VariableExpr>();
    v->name = previous().lexeme;
    return v;
  }
  if (match(TokenType::LPAREN)) {
    // Could be sizeof(type) or (expr)
    if (peek().type == TokenType::INT_KW || peek().type == TokenType::CHAR_KW ||
        peek().type == TokenType::BOOL_KW || peek().type == TokenType::STR_KW) {
      parse_error("sizeof not implemented here");
      return std::make_unique<IntegerExpr>();
    }
    auto e = expression();
    consume(TokenType::RPAREN, "expected ')'");
    return e;
  }
  parse_error("expected expression");
  return std::make_unique<IntegerExpr>();
}
