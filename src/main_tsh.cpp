#include <tsh.h>

using namespace std;

int main() {
  char cmd[81];
  simple_shell *shell = new simple_shell();
  
  cout << "tsh> ";
  while (fgets(cmd, sizeof(cmd), stdin)) {
    CmdTokens *tokens = new CmdTokens();
    if (cmd[0] != '\n') {
      shell->parse_command(cmd, &tokens);
      
      if (shell->isQuit(tokens->cmd[0])) {                                                 
        exit(0);
      } else if (shell->isPrintf(tokens->cmd[0])) {
	shell->printf_command(tokens->cmd);
	cout << endl;
      } else if (shell->isHelp(tokens->cmd[0])) {
	shell->help_command();
      } else if (shell->isAlias(tokens->cmd[0])) {
	shell->alias_command(tokens->cmd);
      } else {
	// only this command supports pipe commands
        shell->exec_command(tokens);
      }
      
    }
    cout << "tsh> ";
  }
  cout << endl;
  exit(0);

}
