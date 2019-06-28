#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<signal.h>
#include <fcntl.h>
#include <sys/wait.h>

//global variables
int pids[512];
int background= 0;
int termSignal=-1;
int exitStatus=0;
int redirection=0;
int foregroundOnly;

/*This method implements the functionality for the cd command*/
void cd(char* path)
{
  //variables
  int ret;
  char* buf = malloc(2048 * sizeof(char));
  char* p= malloc(2048 * sizeof(char));
  //no args
  if(path==NULL)
  {
    ret = chdir(getenv("HOME"));
    if(ret!=0)
    {
      printf("Error changing HOME directory");
    }
    exitStatus = 1;
    fflush(stdout);
   }
   //absolute path
   else if(path[0]=='/')
   {
     ret = chdir(path);
     if(ret!=0)
     {
       printf("Error changing directory to: %s\n", path); 
     }
        
     fflush(stdout);
    }
    //relative path
    else
    {
      buf= get_current_dir_name();
      sprintf(p,"%s/%s",buf,path);
      if(chdir(p)!=0)
      {
        printf("Error changing directory to: %s\n", buf);
        fflush(stdout);
      }
    }
    free(buf);
    free(p);
}
/*This method implements the functionality for the status command*/
void status()
{
  if(exitStatus!=0)
  {
    printf("Exit value:%d",exitStatus);
    fflush(stdout);
  }
  else if(termSignal>0)
  {
    printf("Terminated by Signal %d",termSignal);
    fflush(stdout);
  }
  else
  {
    printf("ExitValue:%d",exitStatus);
    fflush(stdout);
  }

}
/*This method exits shell, kills  all other processes, kills itself */
void eFunction()
{
  int m;
  //loop through pids of background processes
  for(m=0;m<512;m++)
  {
    //kill each one
    if(pids[m]!='\0')
    {
      kill (pids[m],SIGKILL);
    }
  }
    exit(0);
}
/*This method handles the CTRL-Z foreground only mode*/
void catchSIGTSTP(int signo)
{
  //toggle background every time this signal is called
  if(foregroundOnly==0)
  {
    printf("Entering foreground-only mode (& is now ignored)");
    fflush(stdout);
    background=1;
    foregroundOnly=1;
   }
   else if(foregroundOnly==1)
   {
     printf("Exiting foreground-only mode");
     fflush(stdout);
     foregroundOnly=0;
   }
 }
 int main()
 {
     //counters
     int i;
     int l;
     int j=0;
     int k=0;
     
     //control flow variables
     foregroundOnly=0;
     int childExitMethod;
     exitStatus=0;
     termSignal=0;
     
     //signal stuff
     //ignore signal for sigint(CTRL-C)
     struct sigaction SIGINT_action = {0};
     struct sigaction SIGTSTP_action ={0};
     SIGINT_action.sa_handler = SIG_IGN;
     //catch signal for sigtstp
     SIGTSTP_action.sa_handler = catchSIGTSTP;
     sigaction(SIGINT, &SIGINT_action,NULL);
     sigaction(SIGTSTP, &SIGTSTP_action,NULL); 
         
     //user input variables
     int numCharsEntered= -5; 
     size_t bufferSize= 0; 
     char* lineEntered= NULL;
     lineEntered = malloc(2048 * sizeof(char)); 
     char* lineArgs[512];   
     char* token=malloc(2048 * sizeof(char));
     char* temp = malloc(2048 * sizeof(char));
     
     //stdin/stdout variables
     int stdin_redirected = 0;
     int stdout_redirected = 0;
     int dev_null_in;
     int dev_null_out;
     int file_descriptor;
     int file_descriptor2;
     int result1;
     int result2;
        
     char* outFile = malloc(20 * sizeof(char));
     char* inFile = malloc(20 * sizeof(char));
        
     pid_t spawnpid= -5;
     pid_t pid= -5;
     memset(pids,0, 512);
        
     //Get input from the user
     while(1)
     {
       //shell prompt
       printf(": ");
       fflush(stdout);
       //get line from user
       numCharsEntered = getline(&lineEntered, &bufferSize, stdin);
       if(strcmp(lineEntered,"\n")==0)
       {
         continue;
       }
       lineEntered[numCharsEntered-1]='\0';
       //blank line
       if(lineEntered[0]=='#' )
       {
         continue;
       }
       //expand into PID
       if(strstr(lineEntered,"$$")!=NULL)
       {
         temp = strtok(lineEntered,"$$");
         pid= getpid();
         sprintf(lineEntered,"%s%d",temp,pid);
       }
       i=0;
       token= strtok(lineEntered," ");
       lineArgs[i]= token;
       i++;
       //get the rest of the command line
       while((token= strtok(NULL," "))!=NULL)
       {
         lineArgs[i]=token;
         i++;
       }
       //add null to the end
       lineArgs[i]=NULL;
       //check for built in commands
       if(strcmp(lineArgs[0],"cd")==0)
       {
         // next token contains path
         cd(lineArgs[1]);
       }
       else if(strcmp(lineArgs[0],"status")==0)
       {
         status();
       }
       else if(strcmp(lineArgs[0],"exit")==0)
       {
         eFunction();
       }
       //check for background
       else 
       {
             if ((strcmp(lineArgs[i-1], "&") == 0 )) 
             {
               if(foregroundOnly==0)
               {
                 background = 1;
               }
               else
               {
                 background=0;
               }
               //remove ampersand
               lineArgs[i-1]=NULL;
             }
             //fork off a child
             spawnpid = fork();
             j++; //increment number of forks
             if(spawnpid==-1)
             {
               printf("error, child not created");
               fflush(stdout);
               exit(0);
              }
              else if(spawnpid==0)
              {
                if((strcmp(lineArgs[i-2],">")==0) ||(strcmp(lineArgs[i-2],"<")==0))
                {
                   redirection=1;
                   //check for input redirection
                   if(strcmp(lineArgs[1],"<")==0) 
                   {
                     inFile = lineArgs[2];
                     file_descriptor=0;
                     file_descriptor = open(inFile, O_RDONLY);
                     //file opening error handling
                     if (file_descriptor < 0) 
                     {
                       printf("Cannot open%s for input\n", inFile);
                       exitStatus=1;
                     }
                     //redirect stdin
                     result1 = dup2(file_descriptor, 0);
                     //eror handling for dup2
                     if (result1 == -1) 
                     {
                        printf("file_descriptor dup2() falied for input");
                        exitStatus=1;
                      }
                      stdin_redirected = 1;
                     }
                     //check for output redirection
                     if(strcmp(lineArgs[i-2],">")==0)
                     {
                       outFile = lineArgs[i-1];
                       file_descriptor2= 0;
                       fflush(stdout);
                       file_descriptor2 = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                       //error handling for file opening
                       if (file_descriptor2 == -1)
                       {
                         printf("Cannot open%s for output\n", outFile);
                         exitStatus=1;
                       }
                       //redirect stdout
                       result2 = dup2(file_descriptor2, 1);
                       //eror handling for dup2
                       if (result2 == -1)
                       { 
                         printf("file_dscriptor2 dup2() failed for output");
                         exitStatus=1;
                        }
                        stdout_redirected = 1;
                      }
                    }
                    if(background == 1)
                    {
                      // run in background
                      printf("Process ID:(%d) starting in the background",getpid());
                      fflush(stdout);
                      // check if stdin redirected; if not, redirect to /dev/null
                      if (stdin_redirected == 0) 
                      {
                        dev_null_in = open("/dev/null", O_RDONLY);
                        dup2(dev_null_in, 0);
                        stdin_redirected = 1;
                      }
                      // check if stdout redirected; if not, redirect to /dev/null
                      if (stdout_redirected == 0) 
                      {
                        dev_null_out = open("/dev/null", O_WRONLY);
                        dup2(dev_null_out, 1);
                        stdout_redirected = 1;
                      }
                      execvp(lineArgs[0], lineArgs);
                      if(exitStatus==1)
                      {
                          printf("exec failed");
                          exit (1);
                      }
                    }
                    //run in foreground
                    else 
                    {
                      //kill process in foreground upon receipt of a signal
                      SIGINT_action.sa_handler = SIG_DFL;
                      sigaction(SIGINT, &SIGINT_action, NULL);
                      if(redirection==1)
                      {
                        int x;
                        for(x=1;x<i;x++)
                        {
                          lineArgs[x]=NULL;
                        }
                        redirection=0;
                        execvp(lineArgs[0], lineArgs);
                        exitStatus=1;
                        if(exitStatus==1)
                        {
                          printf("exec failed");
                          exit (1);
                        }
                       }
                       else 
                       {
                         execvp(lineArgs[0],lineArgs);
                         exitStatus=1;
                         if(exitStatus==1)
                         {
                            printf("exec failed");
                            exit (1);
                          }
                          printf("No such file or directory");
                        }
                      }
                   //parent
                  }
                  if(background==1)
                  {
                     pids[k]=spawnpid;
                     k++;
                  }
                  else if(background == 0)
                  {
                    //wait for foreground commands to finish
                    waitpid(spawnpid, &childExitMethod,WCONTINUED);
                    // if the child foreground process terminated normally
                    if((WIFEXITED(childExitMethod))!=0)
                    {
                      //get and print out the exit status of the terminated child
                      exitStatus=WEXITSTATUS(childExitMethod);
                    }
                    // child foreground process terminated by signal
                    else if (WIFSIGNALED(childExitMethod) != 0)
                    {
                      printf("The process was terminated by a signal\n");
                      termSignal = WTERMSIG(childExitMethod);
                    }
                  }
             }
             for(l=0;l<512;l++)
             {
               //check if any background processes have completed yet
               if(pids[l]!=0 && (waitpid(pids[l], &childExitMethod, WNOHANG)!=0))
               {
                 printf("Completed PID: %d", pids[l]);
                 fflush(stdout);
                 if (WIFSIGNALED(childExitMethod) != 0)
                 {
                   termSignal = WTERMSIG(childExitMethod);
                   printf("The process was terminated by signal %d\n", termSignal);
                 }
                 else
                 {
                   exitStatus=WEXITSTATUS(childExitMethod);
                   printf("Exit Value: %d");
                 }
                 pids[l]=0;
               }
            }
            background=0;
     }
     for(i=0;i<512;i++)
     {
       free(lineArgs[i]);
     }
     free(lineEntered);
     free(token);
     free(temp);
     free(inFile);
     free(outFile);
   }
            
