#ifndef _SIMPLE_SHELL_H
#define _SIMPLE_SHELL_H
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <list>

typedef struct CmdTokens CmdTokens;
struct CmdTokens {
  char** cmd;
  CmdTokens* pipe;
};

typedef struct Pair Pair;
struct Pair {
  char* name;
  char* value;
};

class simple_shell {
 public:
  std::list<Pair> pairs;
  void parse_command(char* cmd, CmdTokens** tokens);
  void exec_command(CmdTokens* tokens);
  void printf_command(char** cmdTokens, ...);
  void help_command();
  void alias_command(char** cmdTokens);
  void parse_alias_command(char* cmd, char** tokens);
  bool isQuit(char* cmd);
  bool isHelp(char* cmd);
  bool isPrintf(char* cmd);
  bool isAlias(char* cmd);
};

#endif
