#include <stdio.h>
#include <tsh.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <stdarg.h>

using namespace std;

void simple_shell::parse_command(char* cmd, CmdTokens** tokens) {
  // Tokenize the command string into arguments     

  // Checks if cmd is valid 
  size_t length = strlen(cmd);
  if (length > 0 && cmd[length - 1] == '\n') {
    cmd[length - 1] = '\0';
  }

  // Splits cmd via whitespace
  char *temp = strtok(cmd, " ");
  char **cmdTokens = (char**) malloc(sizeof(char));

  int i = 0;
  CmdTokens* curr = *tokens;
  while (temp != NULL) {
    // If it finds pipe
    if (strcmp(temp, "|") == 0) {
      // Finalizing data
      cmdTokens[i] = NULL;
      curr->cmd = cmdTokens;
      // Creating new struct & continueing to finalize data
      CmdTokens *t = (CmdTokens*) malloc(sizeof(CmdTokens*));
      curr->pipe = t;
      // resetting data
      curr = t;
      cmdTokens = (char**) malloc(sizeof(char));
      i = 0;
      temp = strtok(NULL, " ");
    } else {
      cmdTokens[i++] = temp;
      temp = strtok(NULL, " ");
    }
  }
  // Finalizing to final struct
  cmdTokens[i] = NULL;
  curr->cmd = cmdTokens;
}

void simple_shell::exec_command(char** argv1, char** argv2) {
  // Fork a child process to execute the command.
  // Then return and pass result as input to the parent process.

  int counter = 0;
  for (char **temp = argv2; *temp; ++temp) {
    counter++;
  }
 
  if (counter != 0) {
    int pipefd[2];
    string cmdOp1 = argv1[0];
    string cmdOp2 = argv2[0];
    
    if (pipe(pipefd) == 1) {
      perror("Error: failed to create pipe\n.");
      exit(1);
    }

    // creating the first child to handle argv1
    int pid = fork();
    if (pid < 0) {
      cout << "Fork failed to execute\n.";
      exit(1);
    } else if (pid == 0) {
      // child uses write-end and closes read-end
           
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[0]);
      cout << cmdOp1 << endl;
      execvp(cmdOp1.data(), argv1);

      perror("execvp error in child 1");
      exit(1);
    } else {
      // parent uses read-end and closes write-end

      // creating the second child to handle argv2
      int pid2 = fork();
      if (pid2 < 0) {
	cout << "Fork failed to execute in pid2\n.";
        exit(1);
      } else if (pid2 == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        execvp(cmdOp2.data(), argv2);
	
        perror("execvp error in child 2");
        exit(1);
      } else {
	//parent
	cout << "before" << endl;
	int waiting = waitpid(pid, NULL, 0);
	cout << "between" << endl;
	int waiting2 = waitpid(pid2, NULL, 0);
	cout << "after" << endl;
	return;
      }
    }
  } else {
    string cmdOp1 = argv1[0];
    int pid = fork();
    if (pid < 0) {
      cout << "Fork failed to execute\n.";
      exit(1);  
    } else if (pid == 0) {
      //cout << "In child with 1 argv" << endl;
      execvp(cmdOp1.data(), argv1);

      perror("execvp error in child with 1 argv");
      exit(1);
    } else {
      //cout << "In parent with 1 argv" << endl;
      int waiting = waitpid(pid, NULL, 0);
      kill(pid, SIGQUIT);
    }
  }
}

void simple_shell::printf_command(char**cmdTokens, ...) {
    // Check if there are arguments after "printf"
    if (cmdTokens[1] == nullptr) {
        std::cerr << "printf: missing arguments" << std::endl;
        return;
    }

    // Concatenate arguments into a single format string
    string formatString;
    for (int i = 1; cmdTokens[i] != nullptr; ++i) {
        formatString += cmdTokens[i];
        formatString += " ";
    }
        
    // Call printf with the concatenated format string
    va_list args;
    va_start(args, formatString.c_str());
    vprintf(formatString.c_str(), args);
    va_end(args);
}

void simple_shell::help_command() {
    // Provide information about available commands
    cout << "Welcome to the simple shell!" << endl;
    cout << "printf to print" << endl;
    cout << "  help - Display this help message" << endl;
    cout << "  quit - Exit the shell" << endl;
    // Add information about other built-in commands if needed
}

bool simple_shell::isQuit(char* cmd) {
  // Check for the command "quit" that terminates the shell
  string cmdStr = cmd;
  string quitCommand = "quit";
  return (cmdStr.compare(quitCommand) == 0);
}

bool simple_shell::isHelp(char* cmd){
  // check for the command  "help", shell will reponse accordingly
  string cmdStr = cmd;
  string helpCommand = "help";
  return (cmdStr.compare(helpCommand)==0);
}

bool simple_shell::isPrintf(char* cmd){
  // check for the command  "printf", shell will reponse accordingly
  string cmdStr = cmd;
  string PrintfCommand = "printf";
  return (cmdStr.compare(PrintfCommand)==0);
}
