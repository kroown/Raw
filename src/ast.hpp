#pragma once
#include <memory>
#include <string>
#include <vector>

struct Type {
  enum Kind { INT, CHAR, BOOL, STR, VOID, PTR, STRUCT };
  Kind kind = INT;
  int ptr_depth = 0;
  int array_size = 0;
  std::string struct_name;
};

struct ASTNode {
  virtual ~ASTNode() = default;
};
struct Expr : ASTNode {};
struct Stmt : ASTNode {};

struct IntegerExpr : Expr { int64_t value; };
struct StringExpr : Expr { std::string value; };
struct BoolExpr : Expr { bool value; };
struct VariableExpr : Expr { std::string name; Type type; };

struct BinaryExpr : Expr {
  enum Op {
    ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, STAR_ASSIGN, SLASH_ASSIGN,
    PLUS, MINUS, STAR, SLASH, MOD,
    EQUAL_EQUAL, BANG_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
    AND, OR
  };
  Op op;
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;
};

struct UnaryExpr : Expr {
  enum Op { NEG, NOT, ADDR, DEREF, PLUS_PLUS, MINUS_MINUS };
  Op op;
  std::unique_ptr<Expr> operand;
};

struct CallExpr : Expr {
  std::string callee;
  std::vector<std::unique_ptr<Expr>> args;
};

struct IndexExpr : Expr {
  std::unique_ptr<Expr> base;
  std::unique_ptr<Expr> index;
};

struct MemberExpr : Expr {
  std::unique_ptr<Expr> base;
  std::string member;
};

struct StructField {
  std::string name;
  Type type;
};

struct StructDecl : ASTNode {
  std::string name;
  std::vector<StructField> fields;
};

struct StructExpr : Expr {
  std::string struct_name;
  std::vector<std::unique_ptr<Expr>> field_values;
};

struct SizeofExpr : Expr {
  std::unique_ptr<Expr> operand;
};

struct VarDeclStmt : Stmt {
  std::string name;
  Type type;
  std::unique_ptr<Expr> initializer;
  bool is_const = false;
};

struct ExprStmt : Stmt {
  std::unique_ptr<Expr> expr;
};

struct ReturnStmt : Stmt {
  std::unique_ptr<Expr> value;
};

struct IfStmt : Stmt {
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> then_branch;
  std::unique_ptr<Stmt> else_branch;
};

struct WhileStmt : Stmt {
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> body;
};

struct ForStmt : Stmt {
  std::unique_ptr<Stmt> init;
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Expr> inc;
  std::unique_ptr<Stmt> body;
};

struct BreakStmt : Stmt {};
struct ContinueStmt : Stmt {};

struct DeferStmt : Stmt {
  std::unique_ptr<Stmt> stmt;
};

struct Block : Stmt {
  std::vector<std::unique_ptr<Stmt>> statements;
};

struct Param {
  std::string name;
  Type type;
};

struct FnDecl : ASTNode {
  std::string name;
  std::vector<Param> params;
  Type return_type;
  std::unique_ptr<Block> body;
};

struct ExternFnDecl : ASTNode {
  std::string name;
  std::vector<Param> params;
  Type return_type;
};

struct Program : ASTNode {
  std::vector<std::unique_ptr<StructDecl>> structs;
  std::vector<std::unique_ptr<FnDecl>> functions;
  std::vector<ExternFnDecl> extern_functions;
};
