#pragma once
#include "ast.hpp"
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class Codegen {
public:
  explicit Codegen(Program* prog);
  std::string generate();
  std::string error_msg() const { return error; }
  bool had_error() const { return errored; }

private:
  Program* prog;
  std::ostringstream asm_;
  std::ostringstream rodata_;
  bool errored = false;
  std::string error;
  int label_count = 0;
  int string_count = 0;
  std::string current_fn;
  int stack_offs = 0;
  std::unordered_map<std::string, int> locals;
  std::vector<std::string> loop_start_labels;
  std::vector<std::string> loop_end_labels;
  bool helpers_emitted = false;
  std::unordered_map<std::string, std::string> string_labels;

  std::string new_label();
  std::string get_string_label(const std::string& content);
  void emit_rodata();
  void emit_helpers();
  void emit_raw(const std::string& s);

  void gen_function(FnDecl* fn);
  void gen_block(Block* block);
  void gen_stmt(Stmt* stmt);
  void gen_var_decl(VarDeclStmt* v);
  void gen_if(IfStmt* s);
  void gen_while(WhileStmt* s);
  void gen_for(ForStmt* s);
  void gen_return(ReturnStmt* s);
  void gen_break(BreakStmt* s);
  void gen_continue(ContinueStmt* s);
  void gen_expr_stmt(ExprStmt* s);
  void gen_expr(Expr* expr);
};
