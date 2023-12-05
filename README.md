# 377Final

Install 

Use cd to go to folder of this program and type "make" to install the program (NOTE: current Makefile is made for a UMass Edlab machine). Once it is installed on your computer, you can type ./tsh_app to get into the shell and use it.

Commands

Printf: 
          Printf prints strings and with 2 more options, %b and %t, it can prints binrary number of the character and the local time. To use the printf, you can go the shell, and call the command with typein printf with                string you like to print out. 
          For example, if you like to print out "hello world", you can type in printf hello world. No "" are nesscery. 
          To use the options, you will need to type in '%' after printf command and then, type the option charater after the '%'. 
          For exmaple, you can type in printf %bd to get the binrary format of the charater d. 


test:    tsh> printf %bd
         001001010110001001100100

         tsh> printf %t
         2023-12-05 17:08:28

      tsh> printf d\nc\nw
      
      d
      
      c
      
      w
tsh> 
          
Help:
          Help is a command you can use as a user guide of this simple shell. 
          To use the help command,  you can go to shell and type help. Shell will print out the infromation to help you to use the shell.

Features:

Infinite pipeline:
          The user can pipe as many commands as they would like. Examples: `tsh> ls | sort | grep t`
