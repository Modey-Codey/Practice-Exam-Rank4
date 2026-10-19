#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

int    picoshell(char **cmds[])
{
    pid_t pid;          // Store child process ID
    int fd[2];          // Pipe file descriptors: fd[0]=read end, fd[1]=write end
    int last_fd = -1;   // Saved read end from previous pipe (for next cmd's input)
    int i = 0;          // Index for current command in the pipeline
    while (cmds[i])
    {
        if (cmds[i + 1] && pipe(fd) == -1)
            return (1);
        pid = fork();
        if (pid == -1)      // Fork failed
        {
            if (cmds[i + 1])
            {
                close(fd[0]);
                close(fd[1]);
            }
            return (1);
        }
        if (pid == 0)
        {
            if (last_fd != -1)
            {
                if (dup2(last_fd, STDIN_FILENO) == -1)
                    exit(1);
                close(last_fd);     // Close original fd after duplication
            }
            if (cmds[i + 1])
            {
                close(fd[0]);       // Child doesn't need read end
                if (dup2(fd[1], STDOUT_FILENO) == -1)
                    exit(1);
                close(fd[1]);       // Close original fd after duplication
            }
            execvp(cmds[i][0], cmds[i]);
            exit(1);
        }
        if (last_fd != -1)
            close(last_fd);
        if (cmds[i + 1])
        {
            close(fd[1]);       // Parent doesn't need write end
            last_fd = fd[0];    // Save read end for next command's input
        }
        i++;    // Move to next command
    }
    while (wait(NULL) > 0)
        ;
    return (0);     // Success
}

int main(void)
{
    char *cmd1[] = {"ls", NULL};
    char *cmd2[] = {"wc", "-l", NULL};

    char **cmds[] = {cmd1, cmd2, NULL};

    return (picoshell(cmds));
}