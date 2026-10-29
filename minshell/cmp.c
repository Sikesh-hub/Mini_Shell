#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "head.h"

char external_commands[153][20];
int last_status = 0;

int main()
{
    system("clear");

    char prompt_string[50] = "Minishell$ ";
    char str[50];
    char *arg[100];

    extract_external_commands(external_commands);

    while (1)
    {
        printf("%s", prompt_string);
        fflush(stdout);

        if (fgets(str, sizeof(str), stdin) == NULL)
        {
            break;
        }

        str[strcspn(str, "\n")] = '\0';

        if (str[0] == '\0')
        {
            continue;
        }

        if (strncmp(str, "PS1=", 4) == 0)
        {
            if (str[4] == '\0' || strchr(str + 4, ' ') != NULL)
            {
                printf("Invalid prompt string\n");
                continue;
            }

            strcpy(prompt_string, str + 4);
            continue;
        }

        char *token;
        int i = 0;

        token = strtok(str, " \t");

        while (token != NULL && i < 99)
        {
            arg[i] = token;
            i++;

            token = strtok(NULL, " \t");
        }

        arg[i] = NULL;

        if (arg[0] == NULL)
        {
            continue;
        }

        int ret = check_command_type(arg[0]);

        if(ret == BUILTIN)
        {
            execute_internal_command(arg);
        }
        else if(ret == EXTERNAL)
        {
            pid_t pid = fork();

            if (pid == 0)
            {
                execvp(arg[0], arg);
                perror("execvp");
                exit(127);
            }
            else if (pid > 0)
            {
                int status;

                waitpid(pid, &status, 0);

                if (WIFEXITED(status))
                {
                    last_status = WEXITSTATUS(status);
                }
                else
                {
                    last_status = 1;
                }
            }
            else
            {
                perror("fork");
                last_status = 1;
            }
        }
        else
        {
            printf("Command not found: %s\n", arg[0]);
            last_status = 127;
        }
    }

    return 0;
}

void extract_external_commands(char external_commands[][20])
{
    char line[50];
    char *token;
    int i = 0;

    FILE *fp = fopen("extenal.txt", "r");

    if (fp == NULL)
    {
        perror("extenal.txt");
        return;
    }

    while (i < 152 && fgets(line, sizeof(line), fp) != NULL)
    {
        token = strtok(line, " \t\r\n");

        if (token != NULL)
        {
            strncpy(external_commands[i], token, 19);
            external_commands[i][19] = '\0';
            i++;
        }
    }

    fclose(fp);
}

int check_command_type(char *command)
{
    char *builtins[] = {"echo", "cd", "pwd", "exit", NULL};
    for (int i = 0; builtins[i] != NULL; i++)
    {
        if (strcmp(builtins[i], command) == 0)
        {
            return BUILTIN;
        }
    }
    for (int i = 0; i < 153; i++)
    {
        if (external_commands[i][0] == '\0')
        {
            break;
        }

        if (strcmp(external_commands[i], command) == 0)
        {
            return EXTERNAL;
        }
    }
    return NO_COMMAND;
}

void execute_internal_command(char **arg)
{
    if (strcmp(arg[0], "cd") == 0)
    {
        char *path = arg[1];

        if (path == NULL)
        {
            path = getenv("HOME");
        }

        if (path != NULL && chdir(path) != 0)
        {
            perror("cd");
            last_status = 1;
        }
        else
        {
            last_status = 0;
        }
    }
    else if (strcmp(arg[0], "pwd") == 0)
    {
        char cwd[1024];

        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
            last_status = 0;
        }
        else
        {
            perror("pwd");
            last_status = 1;
        }
    }
    else if (strcmp(arg[0],"exit") == 0)
    {
        exit(0);
    }
    else if (strcmp(arg[0], "echo") == 0)
    {
        if (arg[1] != NULL &&
            strcmp(arg[1], "$SHELL") == 0)
        {
            char *shell = getenv("SHELL");

            if (shell != NULL)
            {
                printf("%s", shell);
            }
        }
        else if (arg[1] != NULL &&
                 strcmp(arg[1], "$$") == 0)
        {
            printf("%d", getpid());
        }
        else if (arg[1] != NULL &&
                 strcmp(arg[1], "$?") == 0)
        {
            printf("%d", last_status);
        }
        else
        {
            for (int i = 1; arg[i] != NULL; i++)
            {
                if (i > 1)
                {
                    printf(" ");
                }

                printf("%s", arg[i]);
            }
        }

        printf("\n");
        last_status = 0;
    }
}