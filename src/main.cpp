#include "lexer.hpp"
#include "parser.hpp"
#include "codegen.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

[[noreturn]] void die(const std::string& msg) {
  std::cerr << "rawc: error: " << msg << "\n";
  std::exit(1);
}

std::string read_file(const std::string& path) {
  std::ifstream f(path);
  if (!f) die("cannot open '" + path + "'");
  std::stringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

std::string base_name(const std::string& path) {
  size_t dot = path.rfind('.');
  size_t slash = path.rfind('/');
  if (slash == std::string::npos) slash = 0; else slash++;
  std::string name = path.substr(slash);
  if (dot != std::string::npos) name = name.substr(0, dot);
  return name;
}

void write_file(const std::string& path, const std::string& content) {
  std::ofstream f(path);
  if (!f) die("cannot write '" + path + "'");
  f << content;
}

int main(int argc, char** argv) {
  if (argc < 2) die("usage: rawc <input.raw> [-o output]");

  std::string input_path;
  std::string output_path;

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "-o" && i + 1 < argc) {
      output_path = argv[++i];
    } else {
      input_path = arg;
    }
  }

  if (input_path.empty()) die("no input file specified");
  if (output_path.empty()) output_path = base_name(input_path);

  std::string source = read_file(input_path);

  // tokenize
  Lexer lex(source);
  auto tokens = lex.tokenize();

  // parse
  Parser parser(tokens);
  auto prog = parser.parse();
  if (parser.had_error()) die(parser.error_msg());

  // codegen
  Codegen cg(prog.get());
  std::string asm_text = cg.generate();
  if (cg.had_error()) die(cg.error_msg());

  // write .s
  std::string asm_path = output_path + ".s";
  write_file(asm_path, asm_text);

  // assemble
  std::string obj_path = output_path + ".o";
  std::string as_cmd = "as " + asm_path + " -o " + obj_path;
  int ret = std::system(as_cmd.c_str());
  if (ret != 0) die("assembly failed");

  // link
  std::string ld_cmd = "ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 " + obj_path + " -lc -lSDL2 -o " + output_path;
  ret = std::system(ld_cmd.c_str());
  if (ret != 0) die("linking failed");

  std::cout << output_path << "\n";
  return 0;
}
