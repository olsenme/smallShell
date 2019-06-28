
<h1>Overview</h1>
This is my shell in C. The shell runs command line instructions and returns the results similar to other shells, but without many of their fancier features.
The name of the shell is called smallsh. It works like the bash shell, prompting for a command line and running commands, but it does not have many of the special features of the bash shell.
The shell allows for the redirection of standard input and standard output and it supports both foreground and background processes (controllable by the command line and by receiving signals).
The shell supports three built in commands: exit, cd, and status. It also supports comments, which are lines beginning with the # character.

<h1>Specifications</h1>
<h2>The Prompt</h2>
The colon : symbol is a prompt for each command. 
The general syntax of a command line is:
command [arg1 arg2 ...] [< input_file] [> output_file] [&]
…where items in square brackets are optional.  I assume that a command is made up of words separated by spaces. The special symbols <, >, and & are recognized, but they must be surrounded by spaces like other words. If the command is to be executed in the background, the last word must be &. If standard input or output is to be redirected, the > or < words followed by a filename word must appear after all the arguments. Input redirection can appear before or after output redirection.
The shell does not support any quoting; so arguments with spaces inside them are not possible.
The shell support command lines with a maximum length of 2048 characters, and a maximum of 512 arguments. There is no error checking on the syntax of the command line.
The shell allows blank lines and comments. Any line that begins with the # character is a comment line and is ignored (mid-line comments, such as the C-style //, are not supported).  A blank line (one without any commands) also does nothing. The shell just re-prompts for another command when it receives either a blank line or a comment line.
<h2>Command Execution</h2>
Fork(), exec(), and waitpid() are used to execute commands. From a conceptual perspective, the shell runs in this manner: the parent process (the shell) continues running. Whenever a non-built in command is received, the parent fork()s off a child. This child then does any needed input/output redirection before running exec() on the command given. When doing redirection, after using dup2() to set up the redirection, the redirection symbol and redirection destination/source are NOT passed into the following exec command (i.e., if the command given is ls > more, then it does the redirection and then simply passes ls into exec() ).
Note that exec() will fail, and return the reason why, if it is told to execute something that it cannot do, like a program that doesn't exist. In this case, the shell indicates to the user that a command could not be executed, and sets the value retrieved by the built-in status command to 1.
The shell uses the PATH variable to look for non-built in commands, and it allows shell scripts to be executed. If a command fails because the shell could not find the command to run, then the shell prints an error message and sets the exit status to 1.
As above, after the fork() but before the exec(), any input and/or output redirection is done with dup2(). An input file redirected via stdin is opened for reading only; if the shell cannot open the file for reading, it prints an error message and sets the exit status to 1 (but does not exit the shell). Similarly, an output file redirected via stdout is opened for writing only; it is truncated if it already exists or created if it does not exist. If the shell cannot open the output file it prints an error message and sets the exit status to 1 (but doesn't exit the shell).
Both stdin and stdout for a command can be redirected at the same time (see example below).
The program expands any instance of "$$" in a command into the process ID of the shell itself. The shell does not otherwise perform variable expansion. 
<h2>Background and Foreground</h2>
The shell wait()s for completion of foreground commands (commands without the &) before prompting for the next command. If the command given was a foreground command, then the parent shell does NOT return command line access and control to the user until the child terminates. It is recommend to have the parent simply call waitpid() on the child, while it waits.
The shell does not wait for background commands to complete. If the command given was a background process, then the parent returns command line access and control to the user immediately after forking off the child. In this scenario, the parent shell  periodically checks for the background child processes to complete (with waitpid(...NOHANG...)), so that they can be cleaned up, as the shell continues to run and process commands.  A signal handler is used to immediately wait() for child processes that terminate, as opposed to periodically checking a list of started background processes. The time to print out when these background processes have completed is just BEFORE command line access and control are returned to the user, every time that happens. 
Background commands have their standard input redirected from /dev/null if the user did not specify some other file to take standard input from. What happens to background commands that read from standard input if you forget this? Background commands  also do not not send their standard output to the screen (again, redirected to /dev/null).
The shell will print the process id of a background process when it begins. When a background process terminates, a message showing the process id and exit status is printed. There is a check to see if any background processes completed just before prompting for a new command and print this message then. In this way the messages about completed background processes does not appear during other running commands, though the user will have to wait until they complete some other command to see these messages (this is the way the C shell and Bourne shells work; see example below). waitpid() is used to check for completed background processes.
<h2>Signals</h2>
A CTRL-C command from the keyboard sends a SIGINT signal to the parent process and all children at the same time. The SIGINT does not terminate the shell, but only terminates the foreground command if one is running. To do this, I create appropriate signal handlers with sigaction(). The parent does not attempt to terminate the foreground child process when the parent receives a SIGINT signal: instead, the foreground child (if any) terminates itself on receipt of this signal.
If a child foreground process is killed by a signal, the parent immediately prints out the number of the signal that killed it's foreground child process (see the example) before prompting the user for the next command.
Background processes are also not be terminated by a SIGINT signal. They terminate themselves, continue running, or terminate when the shell exits (see below).
A CTRL-Z command from the keyboard sends a SIGTSTP signal to the shell. When this signal is received, the shell displays an informative message (see below) and then enters a state where new commands can no longer be run in the background. In this state, the & operator is simply ignored - and runs all such commands as if they were foreground processes. If the user sends SIGTSTP again, another informative message (see below) is displayed, and then it returns back to the normal condition where the & operator is once again honored, allowing commands to be placed in the background. See the example below for usage and the exact syntax which you must use for these two informative messages.
<h2>Built-in Commands</h2>
The shell supports three built in commands: exit, cd, and status. Input/output redirection is not supported for these built in commands and does not set any exit status. These three built-in commands are the only ones that the shell handles itself - all others are simply passed on to a member of the exec() family of functions as described above.
If the user tries to run one of these built-in commands in the background with the & option, that option is ignored and runs in the foreground anyway (i.e. doesn't display an error, just runs the command in the foreground).
The exit command exits the shell. It takes no arguments. When this command is run, the shell kills any other processes or jobs  started before it terminates itself.
The cd command changes directories.  By itself, it changes to the directory specified in the HOME environment variable (not to the location where smallsh was executed from, unless the shell is located in the HOME directory).  It can also take one argument, the path of the directory to change to. Note that this is a working directory: when smallsh exits, the pwd will be the original pwd when smallsh was launched. The cd command does not support both absolute and relative paths.
The status command prints out either the exit status or the terminating signal of the last foreground process (not both, processes killed by signals do not have exit statuses!).
Example
Here is an example:
$ smallsh
: ls
junk   smallsh    smallsh.c
: ls > junk
 : status
 exit value 0
: cat junk
junk
smallsh
smallsh.c
: wc < junk > junk2
: wc < junk
       3       3      23
: test -f badfile
: status
exit value 1
: wc < badfile
cannot open badfile for input
: status
exit value 1
: badfile
badfile: no such file or directory
: sleep 5
^Cterminated by signal 2
: status &
terminated by signal 2
: sleep 15 &
background pid is 4923
: ps
   PID TTY      TIME CMD
  4923 pts/4    0:00 sleep
  4564 pts/4    0:03 tcsh-6.0
  4867 pts/4    1:32 smallsh
: 
: # that was a blank command line, this is a comment line
:
background pid 4923 is done: exit value 0
: # the background sleep finally finished
: sleep 30 &
background pid is 4941
: kill -15 4941
background pid 4941 is done: terminated by signal 15
: pwd
/nfs/stak/faculty/b/brewsteb/CS344/prog3
: cd
: pwd
/nfs/stak/faculty/b/brewsteb
: cd CS344
: pwd
/nfs/stak/faculty/b/brewsteb/CS344
: echo 5755
5755
: echo $$
5755
: ^C
: ^Z
Entering foreground-only mode (& is now ignored)
: date
Mon Jan  2 11:24:33 PST 2017
: sleep 5 &
: date
Mon Jan  2 11:24:38 PST 2017
: ^Z
Exiting foreground-only mode
: date
Mon Jan  2 11:24:39 PST 2017
: sleep 5 &
background pid is 4963
: date
Mon Jan  2 11:24:39 PST 2017
: exit
$
Grading Method
The actual grading test script used to assign the program is included. The program functions with the grading script, as follows. To run it, place it in the same directory as your compiled shell, chmod it (chmod +x ./p3testscript) and run this command from a bash prompt:
$ p3testscript 2>&1
or
$ p3testscript 2>&1 | more
or
$ p3testscript > mytestresults 2>&1 

Instruction contributions by Ben Brewster
