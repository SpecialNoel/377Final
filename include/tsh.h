#ifndef _SIMPLE_SHELL_H
#define _SIMPLE_SHELL_H
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>

typedef struct CmdTokens CmdTokens;
struct CmdTokens {
  char** cmd;
  CmdTokens* pipe;
};

class simple_shell {
 public:
  void parse_command(char* cmd, CmdTokens** tokens);
  void exec_command(char** argv1, char** argv2);
  void printf_command(char **cmdTokens, ...);
  void help_command();
  bool isQuit(char* cmd);
  bool isHelp(char* cmd);
  bool isPrintf(char* cmd);
};

#endif
