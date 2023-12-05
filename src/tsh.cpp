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

void simple_shell::exec_command(CmdTokens* tokens) {
  // Fork a child process to execute the command.
  // Then return and pass result as input to the parent process.
  char** argv1 = tokens->cmd;

  if (tokens->pipe != NULL) {
    char** argv2 = tokens->pipe->cmd;
    int pipefd[2];

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
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[0]);

      execvp(argv1[0], argv1);
      perror("execvp error in child 1");
      exit(1);
    }

    // creating the second child to handle argv2
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

    // Parent uses neither pipe and reaps children
    close(pipefd[0]);
    close(pipefd[1]);
	  int waiting = waitpid(pid, NULL, 0);
	  int waiting2 = waitpid(pid2, NULL, 0);
	  return;
  } else {
    int pid = fork();
    if (pid < 0) {
      cout << "Fork failed to execute\n.";
      exit(1);
    } else if (pid == 0) {
      // CHILD
      //cout << "In child with 1 argv" << endl;
      execvp(argv1[0], argv1);
      perror("execvp error in child with 1 argv");
      exit(1);
    } else {
      // PARENT
      //cout << "In parent with 1 argv" << endl;
      int waiting = waitpid(pid, NULL, 0);
    }
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
            expand_escape_sequences(cmdTokens[i + 1], expanded);
            formatString.replace(pos, 16, expanded);
        } else  if (formatString[pos + 1] == 't') {

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


void simple_shell::expand_escape_sequences(const std::string& input, std::string& output) {
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

