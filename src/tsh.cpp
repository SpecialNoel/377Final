#include <stdio.h>
#include <tsh.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <stdarg.h> // printf_command()

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
  char **cmdTokens = (char**) malloc(sizeof(char*));

  int i = 0;
  CmdTokens* curr = *tokens;
  while (temp != NULL) {
    // If it finds pipe
    if (strcmp(temp, "|") == 0) {
      // Finalizing data
      cmdTokens[i] = NULL;
      curr->cmd = cmdTokens;
      // Creating new struct & continuing to finalize data
      CmdTokens *t = (CmdTokens*) malloc(sizeof(CmdTokens));
      curr->pipe = t;
      // Resetting data
      curr = t;
      cmdTokens = (char**) malloc(sizeof(char*));
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

void simple_shell::exec_command(CmdTokens* tokens) {
  // Fork a child process to execute the command.
  // Then return and pass result as input to the parent process.
  char** argv1 = tokens->cmd;

  // Handling pipe commands
  if (tokens->pipe != NULL) {
    char** argv2 = tokens->pipe->cmd;

    // Creates a pipe
    int pipefd[2];
    if (pipe(pipefd) == 1) {
      perror("Error: failed to create pipe\n.");
      exit(1);
    }

    // Creating the first child to handle argv1
    int pid = fork();
    if (pid < 0) {
      cout << "Fork failed to execute\n.";
      exit(1);
    } else if (pid == 0) {
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[0]);

      execvp(argv1[0], argv1);
      perror("execvp error in child 1");
      exit(1);
    }

    // Creating the second child to handle argv2
    int pid2 = fork();
    if (pid2 < 0) {
      cout << "Fork failed to execute in pid2\n.";
      exit(1);
    } else if (pid2 == 0) {
      dup2(pipefd[0], STDIN_FILENO);
      close(pipefd[1]);

      execvp(argv2[0], argv2);
      perror("execvp error in child 2");
      exit(1);
    }
    
    // Parent uses neither sides of the pipe and reaps children
    close(pipefd[0]);
    close(pipefd[1]);
    int waiting = waitpid(pid, NULL, 0);
    int waiting2 = waitpid(pid2, NULL, 0);
    return;
  } else {
    // Handling a single command
    int pid = fork();
    if (pid < 0) {
      cout << "Fork failed to execute\n.";
      exit(1);  
    } else if (pid == 0) {
      // CHILD
      execvp(argv1[0], argv1);
      perror("execvp error in child with 1 argv");
      exit(1);
    } else {
      // PARENT
      int waiting = waitpid(pid, NULL, 0);
    }
  }
}

void simple_shell::printf_command(char** cmdTokens, ...) {
  // Check if there are arguments after "printf"
  if (cmdTokens[1] == nullptr) {
    cerr << "printf: missing arguments" << endl;
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

// this function is for the permanent alias, not the temporary one
void simple_shell::alias_command(char** cmdTokens) {
  // Set a shortcut command for another command
  // Syntax: alias [option] [name]='[value]'

  // Print out all existing aliases
  if (cmdTokens[1] == nullptr) {
    cout << "listing begin" << endl;
    for (list<Pair>::iterator it = pairs.begin(); it != pairs.end(); it++) {
      cout << "alias " << (it)->name << "='" << (it)->value << "'"  << endl;
    }
    cout << "listing completed" << endl;
  } else {
    // Parse alias command
    char** tokens = (char**)malloc(sizeof(char*));
    parse_alias_command(cmdTokens[1], tokens);
    // Extracting name
    char* name = tokens[0];
    cout << "name: " << name << endl;
    // Extracting value
    int length = strlen(tokens[1]);
    char* value = (char*) malloc(sizeof(char));
    strncpy(value, tokens[1]+1, length-2);
    value[length-1] = '\0';
    cout << "value: " << value << endl;

    // Iterate through pairs to see if name already exists
    // If the name exists, replace the exsiting pair with the input command
    bool indicator = false;
    string nameStr = name;
    for (list<Pair>::iterator it = pairs.begin(); it != pairs.end(); it++) {
      string currentName = (it)->name;
      if (nameStr.compare(currentName) == 0) {
	char* newValue = value;
	it = pairs.erase(it);
	Pair newPair;
	newPair.name = (it)->name;
	newPair.value = newValue;
	pairs.push_back(newPair);
	indicator = true;
	break;
      }
    }

    // The name does not exist, add the input command to pairs
    if (!indicator) {
      Pair newPair;
      newPair.name = name;
      newPair.value = value;
      pairs.push_back(newPair);
      indicator = true;
    }

    cout << "new alias added" << endl;
    
    // execute_command();
  }
}

void simple_shell::parse_alias_command(char* cmd, char** tokens) {
  // Tokenize the command of alias into arguments
  // Checks if cmd is valid   
  size_t length = strlen(cmd);
  if (length > 0 && cmd[length - 1] == '\n') {
    cmd[length - 1] = '\0';
  }

  // Splits cmd via the equal sign
  char *temp = (char* )malloc(sizeof(char));
  temp = strtok(cmd, "=");
  tokens[0] = temp;
  temp = strtok(NULL, "=");
  tokens[1] = temp;
  tokens[2] = NULL;
}

bool simple_shell::isQuit(char* cmd) {
  // Check for the command "quit" that terminates the shell
  string cmdStr = cmd;
  string quitCommand = "quit";
  return (cmdStr.compare(quitCommand) == 0);
}

bool simple_shell::isPrintf(char* cmd){
  // Check for the command "printf", shell will reponse accordingly
  string cmdStr = cmd;
  string printfCommand = "printf";
  return (cmdStr.compare(printfCommand)==0);
}

bool simple_shell::isHelp(char* cmd){
  // Check for the command "help", shell will reponse accordingly   
  string cmdStr = cmd;
  string helpCommand = "help";
  return (cmdStr.compare(helpCommand)==0);
}

bool simple_shell::isAlias(char* cmd){
  // Check for the command "alias", shell will reponse accordingly  
  string cmdStr = cmd;
  string aliasCommand = "alias";
  return (cmdStr.compare(aliasCommand)==0);
}
