#include "codegen.hpp"
#include <cassert>

Codegen::Codegen(Program* prog) : prog(prog) {}

std::string Codegen::new_label() {
  return ".L" + std::to_string(label_count++);
}

std::string Codegen::get_string_label(const std::string& content) {
  auto it = string_labels.find(content);
  if (it != string_labels.end()) return it->second;
  std::string lbl = ".S" + std::to_string(string_count++);
  string_labels[content] = lbl;
  return lbl;
}

void Codegen::emit_rodata() {
  rodata_ << ".section .rodata\n";
  for (auto& [content, lbl] : string_labels) {
    rodata_ << lbl << ": .asciz \"";
    for (char c : content) {
      switch (c) {
        case '\n': rodata_ << "\\n"; break;
        case '\t': rodata_ << "\\t"; break;
        case '\0': rodata_ << "\\0"; break;
        case '"':  rodata_ << "\\\""; break;
        case '\\': rodata_ << "\\\\"; break;
        default:   rodata_ << c;
      }
    }
    rodata_ << "\"\n";
  }
  rodata_ << ".FMT_STR: .asciz \"%s\"\n";
  rodata_ << ".FMT_INT: .asciz \"%d\"\n";
}

void Codegen::emit_raw(const std::string& s) {
  asm_ << s << "\n";
}

void Codegen::emit_helpers() {
  if (helpers_emitted) return;
  helpers_emitted = true;

  emit_raw("");
  emit_raw("# print_int helper (uses libc printf)");
  emit_raw(".global print_int");
  emit_raw("print_int:");
  emit_raw("  push rbp");
  emit_raw("  mov rbp, rsp");
#ifdef _WIN32
  emit_raw("  sub rsp, 32");
  emit_raw("  mov edx, ecx");
  emit_raw("  lea rcx, [rip + .FMT_INT]");
#else
  emit_raw("  mov esi, edi");
  emit_raw("  lea rdi, [rip + .FMT_INT]");
#endif
  emit_raw("  xor eax, eax");
  emit_raw("  call printf");
#ifdef _WIN32
  emit_raw("  add rsp, 32");
#endif
  emit_raw("  leave");
  emit_raw("  ret");
}

std::string Codegen::generate() {
  asm_.str("");
  asm_.clear();
  rodata_.str("");
  rodata_.clear();

  emit_raw(".intel_syntax noprefix");
  emit_raw(".section .text");

  for (auto& fn : prog->functions) gen_function(fn.get());

  emit_helpers();

  emit_rodata();
  emit_raw(rodata_.str());

#ifndef _WIN32
  emit_raw(".section .note.GNU-stack,\"\",@progbits");
#endif
  return asm_.str();
}

void Codegen::gen_function(FnDecl* fn) {
  current_fn = fn->name;
  locals.clear();
  stack_offs = 0;

  emit_raw("");
  emit_raw(".global " + fn->name);
  emit_raw(fn->name + ":");
  emit_raw("  push rbp");
  emit_raw("  mov rbp, rsp");
  emit_raw("  sub rsp, 1024");

  {
#ifdef _WIN32
    int max_param_regs = 4;
    const char* regs[] = {"rcx", "rdx", "r8", "r9"};
#else
    int max_param_regs = 6;
    const char* regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
#endif
    for (size_t i = 0; i < fn->params.size() && i < (size_t)max_param_regs; i++) {
      stack_offs += 8;
      locals[fn->params[i].name] = stack_offs;
      emit_raw(std::string("  mov [rbp - ") + std::to_string(stack_offs) + "], " + regs[i]);
    }
    for (size_t i = max_param_regs; i < fn->params.size(); i++) {
      stack_offs += 8;
      locals[fn->params[i].name] = stack_offs;
    }
  }

  gen_block(fn->body.get());

  emit_raw(".L_return_" + fn->name + ":");
  emit_raw("  leave");
  emit_raw("  ret");
}

void Codegen::gen_block(Block* block) {
  for (auto& stmt : block->statements) gen_stmt(stmt.get());
}

void Codegen::gen_stmt(Stmt* stmt) {
  if      (auto* v = dynamic_cast<VarDeclStmt*>(stmt))   gen_var_decl(v);
  else if (auto* i = dynamic_cast<IfStmt*>(stmt))        gen_if(i);
  else if (auto* w = dynamic_cast<WhileStmt*>(stmt))     gen_while(w);
  else if (auto* f = dynamic_cast<ForStmt*>(stmt))       gen_for(f);
  else if (auto* r = dynamic_cast<ReturnStmt*>(stmt))    gen_return(r);
  else if (auto* bpr = dynamic_cast<BreakStmt*>(stmt))   gen_break(bpr);
  else if (auto* c = dynamic_cast<ContinueStmt*>(stmt))  gen_continue(c);
  else if (auto* e = dynamic_cast<ExprStmt*>(stmt))      gen_expr_stmt(e);
  else if (auto* b = dynamic_cast<Block*>(stmt))         gen_block(b);
}

void Codegen::gen_var_decl(VarDeclStmt* v) {
  int size = v->type.array_size > 0 ? v->type.array_size * 8 : 8;
  stack_offs += size;
  locals[v->name] = stack_offs;
  if (v->initializer) gen_expr(v->initializer.get());
  else emit_raw("  xor rax, rax");
  emit_raw("  mov [rbp - " + std::to_string(stack_offs) + "], rax");
  for (int off = 8; off < size; off += 8) {
    emit_raw("  mov [rbp - " + std::to_string(stack_offs - off) + "], rax");
  }
}

void Codegen::gen_if(IfStmt* s) {
  std::string else_lbl = new_label();
  std::string end_lbl = new_label();
  gen_expr(s->condition.get());
  emit_raw("  cmp al, 0");
  emit_raw("  je " + else_lbl);
  gen_stmt(s->then_branch.get());
  emit_raw("  jmp " + end_lbl);
  emit_raw(else_lbl + ":");
  if (s->else_branch) gen_stmt(s->else_branch.get());
  emit_raw(end_lbl + ":");
}

void Codegen::gen_while(WhileStmt* s) {
  std::string start = new_label();
  std::string end = new_label();
  loop_start_labels.push_back(start);
  loop_end_labels.push_back(end);
  emit_raw(start + ":");
  gen_expr(s->condition.get());
  emit_raw("  cmp al, 0");
  emit_raw("  je " + end);
  gen_stmt(s->body.get());
  emit_raw("  jmp " + start);
  emit_raw(end + ":");
  loop_start_labels.pop_back();
  loop_end_labels.pop_back();
}

void Codegen::gen_for(ForStmt* s) {
  if (s->init) gen_stmt(s->init.get());
  std::string start = new_label();
  std::string end = new_label();
  loop_start_labels.push_back(start);
  loop_end_labels.push_back(end);

  emit_raw(start + ":");
  if (s->condition) {
    gen_expr(s->condition.get());
    emit_raw("  cmp al, 0");
    emit_raw("  je " + end);
  }
  gen_stmt(s->body.get());
  if (s->inc) gen_expr(s->inc.get());
  emit_raw("  jmp " + start);
  emit_raw(end + ":");
  loop_start_labels.pop_back();
  loop_end_labels.pop_back();
}

void Codegen::gen_return(ReturnStmt* s) {
  if (s->value) gen_expr(s->value.get());
  emit_raw("  jmp .L_return_" + current_fn);
}

void Codegen::gen_break(BreakStmt*) {
  if (loop_end_labels.empty()) { errored = true; error = "break outside loop"; return; }
  emit_raw("  jmp " + loop_end_labels.back());
}

void Codegen::gen_continue(ContinueStmt*) {
  if (loop_start_labels.empty()) { errored = true; error = "continue outside loop"; return; }
  emit_raw("  jmp " + loop_start_labels.back());
}

void Codegen::gen_expr_stmt(ExprStmt* s) {
  gen_expr(s->expr.get());
}

void Codegen::gen_expr(Expr* expr) {
  if (auto* i = dynamic_cast<IntegerExpr*>(expr)) {
    emit_raw("  mov rax, " + std::to_string(i->value));
    return;
  }
  if (auto* b = dynamic_cast<BoolExpr*>(expr)) {
    emit_raw("  mov rax, " + std::to_string(b->value ? 1 : 0));
    return;
  }
  if (auto* se = dynamic_cast<StringExpr*>(expr)) {
    std::string lbl = get_string_label(se->value);
    emit_raw("  lea rax, [rip + " + lbl + "]");
    return;
  }
  if (auto* v = dynamic_cast<VariableExpr*>(expr)) {
    auto it = locals.find(v->name);
    if (it != locals.end())
      emit_raw("  mov rax, [rbp - " + std::to_string(it->second) + "]");
    else
      emit_raw("  xor rax, rax");
    return;
  }

  if (auto* sz = dynamic_cast<SizeofExpr*>(expr)) {
    (void)sz;
    emit_raw("  mov rax, 8");
    return;
  }

  if (auto* u = dynamic_cast<UnaryExpr*>(expr)) {
    gen_expr(u->operand.get());
    if (u->op == UnaryExpr::NEG) { emit_raw("  neg rax"); return; }
    if (u->op == UnaryExpr::NOT) {
      emit_raw("  cmp rax, 0");
      emit_raw("  sete al");
      emit_raw("  movzx rax, al");
      return;
    }
    if (u->op == UnaryExpr::ADDR) {
      if (auto* va = dynamic_cast<VariableExpr*>(u->operand.get())) {
        auto it = locals.find(va->name);
        if (it != locals.end())
          emit_raw("  lea rax, [rbp - " + std::to_string(it->second) + "]");
        else
          emit_raw("  xor rax, rax");
      }
      return;
    }
    if (u->op == UnaryExpr::DEREF) {
      emit_raw("  mov rax, [rax]");
      return;
    }
    if (u->op == UnaryExpr::PLUS_PLUS) {
      emit_raw("  add rax, 1");
      if (auto* va = dynamic_cast<VariableExpr*>(u->operand.get())) {
        auto it = locals.find(va->name);
        if (it != locals.end())
          emit_raw("  mov [rbp - " + std::to_string(it->second) + "], rax");
      }
      return;
    }
    if (u->op == UnaryExpr::MINUS_MINUS) {
      emit_raw("  sub rax, 1");
      if (auto* va = dynamic_cast<VariableExpr*>(u->operand.get())) {
        auto it = locals.find(va->name);
        if (it != locals.end())
          emit_raw("  mov [rbp - " + std::to_string(it->second) + "], rax");
      }
      return;
    }
    return;
  }

  if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
    if (bin->op == BinaryExpr::ASSIGN) {
      auto* va = dynamic_cast<VariableExpr*>(bin->left.get());
      if (!va) { errored = true; error = "assign target must be variable"; return; }
      gen_expr(bin->right.get());
      auto it = locals.find(va->name);
      if (it != locals.end())
        emit_raw("  mov [rbp - " + std::to_string(it->second) + "], rax");
      return;
    }

    if (bin->op == BinaryExpr::PLUS_ASSIGN || bin->op == BinaryExpr::MINUS_ASSIGN ||
        bin->op == BinaryExpr::STAR_ASSIGN || bin->op == BinaryExpr::SLASH_ASSIGN) {
      auto* va = dynamic_cast<VariableExpr*>(bin->left.get());
      if (!va) { errored = true; error = "compound assign target must be variable"; return; }
      auto it = locals.find(va->name);
      if (it == locals.end()) return;

      emit_raw("  mov rax, [rbp - " + std::to_string(it->second) + "]");
      emit_raw("  push rax");
      gen_expr(bin->right.get());
      emit_raw("  mov rcx, rax");
      emit_raw("  pop rax");

      switch (bin->op) {
        case BinaryExpr::PLUS_ASSIGN: emit_raw("  add rax, rcx"); break;
        case BinaryExpr::MINUS_ASSIGN: emit_raw("  sub rax, rcx"); break;
        case BinaryExpr::STAR_ASSIGN: emit_raw("  imul rax, rcx"); break;
        case BinaryExpr::SLASH_ASSIGN: emit_raw("  xor rdx, rdx"); emit_raw("  idiv rcx"); break;
        default: break;
      }
      emit_raw("  mov [rbp - " + std::to_string(it->second) + "], rax");
      return;
    }

    gen_expr(bin->left.get());
    emit_raw("  push rax");
    gen_expr(bin->right.get());
    emit_raw("  mov rcx, rax");
    emit_raw("  pop rax");

    switch (bin->op) {
      case BinaryExpr::PLUS:  emit_raw("  add rax, rcx"); break;
      case BinaryExpr::MINUS: emit_raw("  sub rax, rcx"); break;
      case BinaryExpr::STAR:  emit_raw("  imul rax, rcx"); break;
      case BinaryExpr::SLASH: emit_raw("  xor rdx, rdx"); emit_raw("  idiv rcx"); break;
      case BinaryExpr::MOD:   emit_raw("  xor rdx, rdx"); emit_raw("  idiv rcx"); emit_raw("  mov rax, rdx"); break;
      case BinaryExpr::EQUAL_EQUAL:   emit_raw("  cmp rax, rcx"); emit_raw("  sete al"); emit_raw("  movzx rax, al"); break;
      case BinaryExpr::BANG_EQUAL:    emit_raw("  cmp rax, rcx"); emit_raw("  setne al"); emit_raw("  movzx rax, al"); break;
      case BinaryExpr::LESS:          emit_raw("  cmp rax, rcx"); emit_raw("  setl al"); emit_raw("  movzx rax, al"); break;
      case BinaryExpr::LESS_EQUAL:    emit_raw("  cmp rax, rcx"); emit_raw("  setle al"); emit_raw("  movzx rax, al"); break;
      case BinaryExpr::GREATER:       emit_raw("  cmp rax, rcx"); emit_raw("  setg al"); emit_raw("  movzx rax, al"); break;
      case BinaryExpr::GREATER_EQUAL: emit_raw("  cmp rax, rcx"); emit_raw("  setge al"); emit_raw("  movzx rax, al"); break;
      case BinaryExpr::AND:
        emit_raw("  cmp rax, 0");
        emit_raw("  sete al");
        emit_raw("  cmp rcx, 0");
        emit_raw("  setne cl");
        emit_raw("  and al, cl");
        emit_raw("  movzx rax, al");
        break;
      case BinaryExpr::OR:
        emit_raw("  cmp rax, 0");
        emit_raw("  setne al");
        emit_raw("  cmp rcx, 0");
        emit_raw("  setne cl");
        emit_raw("  or al, cl");
        emit_raw("  movzx rax, al");
        break;
      default: break;
    }
    return;
  }

  if (auto* c = dynamic_cast<CallExpr*>(expr)) {
    if (c->callee == "print") {
      if (c->args.empty()) return;
      if (auto* se = dynamic_cast<StringExpr*>(c->args[0].get())) {
        std::string lbl = get_string_label(se->value);
#ifdef _WIN32
        emit_raw("  sub rsp, 32");
        emit_raw("  lea rdx, [rip + " + lbl + "]");
        emit_raw("  lea rcx, [rip + .FMT_STR]");
#else
        emit_raw("  lea rsi, [rip + " + lbl + "]");
        emit_raw("  lea rdi, [rip + .FMT_STR]");
#endif
        emit_raw("  xor eax, eax");
        emit_raw("  call printf");
#ifdef _WIN32
        emit_raw("  add rsp, 32");
#endif
      } else {
        gen_expr(c->args[0].get());
#ifdef _WIN32
        emit_raw("  mov ecx, eax");
#else
        emit_raw("  mov edi, eax");
#endif
        emit_raw("  call print_int");
      }
      return;
    }

    for (int i = static_cast<int>(c->args.size()) - 1; i >= 0; i--) {
      gen_expr(c->args[i].get());
      emit_raw("  push rax");
    }
    for (int i = 0; i < static_cast<int>(c->args.size()) && i < 6; i++) {
#ifdef _WIN32
      static const char* regs[] = {"rcx", "rdx", "r8", "r9"};
#else
      static const char* regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
#endif
      emit_raw(std::string("  pop ") + regs[i]);
    }
    emit_raw("  call " + c->callee);
    return;
  }

  if (auto* idx = dynamic_cast<IndexExpr*>(expr)) {
    gen_expr(idx->base.get());
    emit_raw("  push rax");
    gen_expr(idx->index.get());
    emit_raw("  mov rcx, rax");
    emit_raw("  pop rax");
    emit_raw("  lea rax, [rax + rcx*8]");
    emit_raw("  mov rax, [rax]");
    return;
  }

  if (auto* m = dynamic_cast<MemberExpr*>(expr)) {
    gen_expr(m->base.get());
    return;
  }
}
