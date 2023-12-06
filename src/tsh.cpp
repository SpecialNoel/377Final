#include <stdio.h>
#include <tsh.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <stdarg.h>
#include <string>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <bitset>

using namespace std;

void simple_shell::parse_command(char* cmd, CmdTokens** tokens, int* tokenCount) {
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
      *tokenCount += 1;
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
  *tokenCount += 1;
}

void simple_shell::exec_command(CmdTokens* tokens, int tokenCount) {
  // My tests: 
  // ls
  // ls | sort
  // ls | sort | grep t
  // cat src/tsh.cpp | grep string | grep c | grep f

  CmdTokens* curr = tokens;
  int pipes[tokenCount-1][2];
  int pids[tokenCount];
  for (int i = 0; i < tokenCount; i++) {
    char** args = curr->cmd;
    // TODO: if final one, dont create
    if (i != tokenCount-1) {
      if (pipe(pipes[i]) == 1) {
        perror("Error: failed to create pipe\n.");
        exit(1);
      }
    }
    
    // creating the first child to handle argv1
    pids[i] = fork();
    if (pids[i] < 0) {
      cout << "Fork failed to execute\n.";
      exit(1);
    } else if (pids[i] == 0) {
      if (i != 0) {
        //piping inputs
        close(pipes[i-1][1]);
        dup2(pipes[i-1][0], STDIN_FILENO);
        close(pipes[i-1][0]);
      }

      if (i != tokenCount-1) {
        // Piping outputs
        close(pipes[i][0]);
        dup2(pipes[i][1], STDOUT_FILENO);
        close(pipes[i][1]);
      }

      execvp(args[0], args);
      perror("execvp error in child 1");
      exit(1);
    }

    // closing previous' pipes
    if (i != 0) {
      close(pipes[i-1][0]);
      close(pipes[i-1][1]);
    }

    curr = curr->pipe;
  }

  // Reap the children
  for (int i = 0; i < tokenCount; i++) {
    int waiting = waitpid(pids[i], NULL, 0);
  }
}

void simple_shell::printf_command(char** cmdTokens, ...) {
    // Check if there are arguments after "printf"
    if (cmdTokens[1] == nullptr) {
      std::cerr << "printf: missing arguments" << std::endl;
      return;
    }

    std::string formatString;
    va_list args;
    va_start(args, cmdTokens);

    // Collect arguments into a single format string
    //std::string formatString;
    for (int i = 1; cmdTokens[i] != nullptr; ++i) {
      formatString += cmdTokens[i];
      if (cmdTokens[i + 1] != nullptr) {
        formatString += " ";  // Add space between words
      }
    }

    // Process format extensions
    size_t pos = 0;
    int i =0;
    while ((pos = formatString.find("%", pos)) != std::string::npos) {
      if (formatString[pos + 1] == 'b') {
        // Handle %b - expand backslash escape sequences
        // Note: This assumes that cmdTokens[i + 1] is a string
        std::string expanded;
        binary_get(cmdTokens[i + 1], expanded);
        formatString.replace(pos, 16, expanded);
      } else if (formatString[pos + 1] == 't') {
        //cout << ctime(&timenow) <<  endl;
        std::string timestamp;
        get_current_timestamp(timestamp);
        formatString.replace(pos, 2, timestamp);
        //formatString += ctime(&timenow);
      }
      pos += 1;  // Move to the next position after the processed extension
      i++;
    }

    pos = 0;
    while ((pos = formatString.find("\\", pos)) != std::string::npos) {
      // Handle \n - replace with newline character
      if (formatString[pos + 1] == 'n') {
        formatString.replace(pos, 2, "\n");
      }
      pos += 1;  // Move to the next position after the processed extension
      i++;
    }

    // Call printf with the concatenated format string and variable arguments
    /*va_list args;
    va_start(args, cmdTokens[1]);
    vprintf(formatString.c_str(), args);
    va_end(args);
    */
    vprintf(formatString.c_str(), args);

    va_end(args);
}

void simple_shell::help_command() {
  // Provide information about available commands
  cout << "Welcome to the simple shell!" << endl;
  cout << "  printf to print" << endl;
  cout << "  help - Display this help message" << endl;
  cout << "  echo - Output provided arguments" << endl;
  cout << "  read - Read and store one line from standard input" << endl;
  cout << "  quit - Exit the shell" << endl;
}

void simple_shell::read_command(char** cmdTokens, ...) {
  string formatString;
  for (int i = 1; cmdTokens[i] != nullptr; ++i) {
    formatString += cmdTokens[i];
    formatString += " ";
  }
  // Store the provided read variable in the second value of read_line pair in simple_shell class
  read_line.second = formatString;
  // Prompt user for standard input, stored in the first value of read_line pair in simple_shell class
  getline(cin, read_line.first);
}

void simple_shell::echo_command(char** cmdTokens, ...) {
  // Check if there are arguments after "echo"
  if (cmdTokens[1] == nullptr) {
    std::cerr << "echo: missing arguments" << std::endl;
    return;
  }

  // Concatenate arguments into a single format string
  string formatString;
  for (int i = 1; cmdTokens[i] != nullptr; ++i) {
    formatString += cmdTokens[i];
    formatString += " ";
  }
  // Check if the echo argument is a stored variable --> print stored variable,
  // "$REPLY" used if no read variable was provided. Else print out provided argument.
  if((formatString == "$" + read_line.second || formatString == "$REPLY ") && formatString[0] == '$'){
    cout << read_line.first << endl;
  } else {
    va_list args;
    va_start(args, formatString.c_str());
    vprintf(formatString.c_str(), args);
    va_end(args);
  }
}

bool simple_shell::isQuit(char* cmd) {
  // Check for the command "quit" that terminates the shell
  string cmdStr = cmd;
  string quitCommand = "quit";
  return (cmdStr.compare(quitCommand) == 0);
}

bool simple_shell::isHelp(char* cmd){
  // check for the command "help", shell will reponse accordingly
  string cmdStr = cmd;
  string helpCommand = "help";
  return (cmdStr.compare(helpCommand)==0);
}

bool simple_shell::isPrintf(char* cmd){
  // check for the command "printf", shell will reponse accordingly
  string cmdStr = cmd;
  string PrintfCommand = "printf";
  return (cmdStr.compare(PrintfCommand)==0);
}

bool simple_shell::isRead(char* cmd) {
  // check for "read" command
  string cmdStr = cmd;
  string readCommand = "read";
  return(cmdStr.compare(readCommand) == 0);
}

bool simple_shell::isEcho(char* cmd) {
  // check for "echo" command
  string cmdStr = cmd;
  string echoCommand = "echo";
  return(cmdStr.compare(echoCommand) == 0);
}

void simple_shell::binary_get(const std::string& input, std::string& output) {
  output.clear();  // Clear the output string before appending characters
  std::ostringstream oss;
  for (size_t i = 0; i < input.size(); ++i) {
    // Check for escape sequences
    std::bitset<8>binaryRepresentation(input[i]);
    oss << binaryRepresentation.to_string();
  }
  output = oss.str();
}

void simple_shell::get_current_timestamp(std::string& output) {
  // Get the current timestamp in seconds since the epoch
  time_t current_time = std::time(nullptr);

  // Format the current timestamp using strftime
  char buffer[256]; // Adjust the buffer size as needed
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&current_time));

  // Assign the formatted string to the output
  output = buffer;
  // return output;
}
