#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include "head.h"

int main()
{
    system("clear");
    char prompt_string[50] = "Minishell$ ";
    char str[50];
    //char *arg[100];
    char *arg[20];
    char extena[153][20];
    extract_external_commands(extena);
    while (1)
    {
        printf("%s",prompt_string);
        //scanf(" %[^\n]", str);
        fgets(str, sizeof(str), stdin);
        str[strcspn(str, "\n")] = '\0';

        if(strncmp(str,"PS1=",4)==0)
        {
            for(int i=0;str[i] != '\0';i++)
            {
                if(str[i] ==' ')
                {
                    break;
                }
                
            }
            strcpy(prompt_string,str+4);
            printf("%s",prompt_string);
            //return 0;

        }
        else
        {
            

            int i = 0;
            char *token = strtok(str, " ");

            while(token != NULL)
            {
                arg[i] = token;
                i++;

                token = strtok(NULL, " ");
            }

            arg[i] = NULL;
            int ret=check_command_type(arg[0],extena);
            if(ret==BUILTIN){
                execute_internal_command(arg);
                //break;

            }
            else if(ret == EXTERNAL)
            {
                execute_external_commands(arg);

                //break;
            }
            else
            {
            printf("%s : Command not found\n", arg[0]);
            }
            
        }
        //return 1;

        

    }
    
    //return 1;
}
void extract_external_commands(char external_commands[][20])
{
    char line[50],*token;
    int i = 0;
    FILE *fp = fopen("extenal.txt","r");

     while (fgets(line, sizeof(line), fp) != NULL)
    {
        token = strtok(line," \n");
        if(token != NULL)
        {
            strcpy(external_commands[i],token);
            i++;
        }
    }
    //external_commands[152] == '\0';

    fclose(fp);

}

int check_command_type(char *command,char extena[][20])
{
    char *builtins[] = {"echo","cd", "pwd","exit","pg","bg","Jobs", NULL};

    for(int i=0;i <152;i++)
    {
        if(strcmp(extena[i],command)==0)
        {
            return EXTERNAL;
        }
    }

    for(int i=0;builtins[i] !=NULL;i++)
    {
        if(strcmp(builtins[i],command)==0)
        {
            return BUILTIN;
        }
    }

    return NO_COMMAND;

    
}

void execute_internal_command(char **arg)
{
    if(strcmp(arg[0] ,"cd")==0)
    {
        char cwd[200];
        if (chdir(arg[1]) == 0)
        {
            getcwd(cwd, sizeof(cwd));
            printf("%s\n", cwd);
        }
        else
        {
            perror("cd");
        }
    }
    else if(strcmp(arg[0],"pwd")==0)
    {
        char cwd[1024];

        if(getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
        }

    }
    //hi
    else if(strcmp(arg[0],"exit")==0)
    {
        exit(0);
    }
    else if(strcmp(arg[0],"echo")==0)
    {
        if(strcmp(arg[1],"$SHILL")==0)
        {
            printf("/bin/bash\n");
        }
        else if(strcmp(arg[1],"$$")==0)
        {
            printf("%d\n",getpid());
        }
        else if(strcmp(arg[1],"$?")==0)
        {
            printf("0 is running");
        }
        else
        {
             
            printf("%s\n", arg[1]);
            
        }
        
    }
    /*else if(strcmp(arg[1],"fg")==0)
    {

    }
    else if(strcmp(arg[1],"pg")==0)
    {

    }
    else if(strcmp(arg[1],"job")==0)
    {

    }*/
    


}


void  execute_external_commands(char **args)
{
   
  int pipe_count = 0;
  int args_size = 0;
  for(int i=0 ; args[i]!=NULL;i++)
  {
     args_size++;
  }
  int pipe_pos[args_size];
  for(int i = 0 ; i< args_size;i++)
  {
    if(strcmp(args[i],"|") == 0)
    {
         if (i == 0)
        {
            printf("Error: Missing command before pipe\n");
            return ;
        }
        if (i == args_size - 1)
        {
            printf("Error: Missing command after pipe\n");
            return ;
        }
        if (strcmp(args[i - 1], "|") == 0)
        {
            printf("Error: Consecutive pipes are not allowed\n");
            return ;
        }
           pipe_pos[pipe_count] = i;
           pipe_count++;
    }
  }

  if(pipe_count == 0)
  {
     execvp(args[0] , args);
     exit(0);
  }
  else
  {
    for (int i = 0; i < pipe_count; i++)
    {
        args[pipe_pos[i]] = NULL;
    }

    int fd[pipe_count][2];
    for(int i =0 ;i<pipe_count;i++)
    {
        if(pipe(fd[i]) == -1)
        {
            perror("pipe");
            return ;
        }
    }

    pid_t pid;
    int start = 0;

    for (int i = 0; i <= pipe_count; i++)
    {
        pid = fork();

        if (pid == -1)
        {
            perror("fork");
            return ;
        }

        if (pid == 0)
        {
            if (i > 0)
            {
                if (dup2(fd[i - 1][0], STDIN_FILENO) == -1)
                {
                    perror("dup2 input");
                    _exit(1);
                }
            }
            if (i < pipe_count)
            {
                if (dup2(fd[i][1], STDOUT_FILENO) == -1)
                {
                    perror("dup2 output");
                }
            }
            for (int j = 0; j < pipe_count; j++)
            {
                close(fd[j][0]);
                close(fd[j][1]);
            }

            execvp(args[start], &args[start]);

        }
        if (i < pipe_count)
        {
            start = pipe_pos[i] + 1;
        }
    }
    for (int i = 0; i < pipe_count; i++)
    {
        close(fd[i][0]);
        close(fd[i][1]);
    }
    for (int i = 0; i <= pipe_count; i++)
    {
        wait(NULL);
    }
    }


  }