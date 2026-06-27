#include "lexer.hpp"
#include "parser.hpp"
#include "codegen.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

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
#ifdef _WIN32
  size_t slash = path.rfind('\\');
  if (slash == std::string::npos) slash = path.rfind('/');
#else
  size_t slash = path.rfind('/');
#endif
  if (slash == std::string::npos) slash = 0; else slash++;
  std::string name = path.substr(slash);
  if (dot != std::string::npos && dot > slash) name = name.substr(0, dot);
  return name;
}

void write_file(const std::string& path, const std::string& content) {
  std::ofstream f(path);
  if (!f) die("cannot write '" + path + "'");
  f << content;
}

#ifdef _WIN32
std::string find_gcc() {
  // check PATH first
  char* path_env = nullptr;
  size_t len = 0;
  if (_dupenv_s(&path_env, &len, "PATH") == 0 && path_env) {
    std::string paths(path_env);
    free(path_env);
    size_t start = 0;
    while (true) {
      size_t sep = paths.find(';', start);
      std::string dir = paths.substr(start, sep - start);
      std::string candidate = dir + "\\gcc.exe";
      std::ifstream test(candidate);
      if (test) return "gcc";
      if (sep == std::string::npos) break;
      start = sep + 1;
    }
  }
  // common MinGW locations
  const char* locations[] = {
    "C:\\msys64\\mingw64\\bin\\gcc.exe",
    "C:\\msys64\\ucrt64\\bin\\gcc.exe",
    "C:\\msys64\\clang64\\bin\\gcc.exe",
    "C:\\mingw64\\bin\\gcc.exe",
    "C:\\mingw32\\bin\\gcc.exe",
    nullptr
  };
  for (int i = 0; locations[i]; i++) {
    std::ifstream test(locations[i]);
    if (test) return std::string(locations[i]);
  }
  return "";
}
#endif

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

  Lexer lex(source);
  auto tokens = lex.tokenize();

  Parser parser(tokens);
  auto prog = parser.parse();
  if (parser.had_error()) die(parser.error_msg());

  Codegen cg(prog.get());
  std::string asm_text = cg.generate();
  if (cg.had_error()) die(cg.error_msg());

  std::string asm_path = output_path + ".s";
  write_file(asm_path, asm_text);

  std::string exe_path = output_path;
#ifdef _WIN32
  exe_path += ".exe";
  std::string gcc_path = find_gcc();
  if (gcc_path.empty()) die("gcc not found. install MinGW (https://www.mingw-w64.org) or MSYS2 (https://www.msys2.org)");
  std::string cmd = gcc_path + " " + asm_path + " -o " + exe_path;
#else
  std::string cmd = "gcc " + asm_path + " -o " + exe_path;
#endif
  int ret = std::system(cmd.c_str());
  if (ret != 0) die("assembly/linking failed");

  std::cout << exe_path << "\n";
  return 0;
}
