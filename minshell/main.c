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
        scanf("%[^\n]", str);

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
            return 0;

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
            int ret=check_command_type(arg[0]);
            if(ret==BUILTIN){
                execute_internal_command(arg);
                break;

            }
            else if(ret == EXTERNAL)
            {

                break;
            }
            
        }
        return 1;

        

    }
    
    return 1;
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

int check_command_type(char *command)
{
    char *builtins[] = {"echo","cd", "pwd","exit","pg","bg","Jobs", NULL};

    /*for(int i=0;extena[i]<152;i++)
    {
        if(strcmp(extena[i],command)==0)
        {
            return EXTERNAL;
        }
    }*/

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
            //printf(" ");
        }
        else
        {
             
            printf("%s\n", arg[1]);
            
        }
    }
    


}