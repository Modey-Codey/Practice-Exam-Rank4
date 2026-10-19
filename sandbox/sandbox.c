#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

void alarm_handler(int sig)
{
    (void)sig;  // Unused parameter, avoid compiler warning
}

int sandbox(void (*f)(void), unsigned int timeout, bool verbose)
{
    struct sigaction sa;    // Structure to configure signal handler
    pid_t pid;              // Will store child process ID
    int status;             // Will store child's exit status

    sa.sa_handler = alarm_handler;  // Set our handler function
    sa.sa_flags = 0;                // No special flags
    sigemptyset(&sa.sa_mask);       // Don't block any signals during handler
    sigaction(SIGALRM, &sa, NULL);  // Register the handler for SIGALRM

    pid = fork();
    if (pid == -1)          // Fork failed
        return (-1);
    
    if (pid == 0)
    {
        f();        // Execute the test function (might crash, hang, etc.)
        exit(0);    // If f returns normally, exit with success code
    }
    
    alarm(timeout);
    
    if (waitpid(pid, &status, 0) == -1)
    {
        if (errno == EINTR)
        {
            kill(pid, SIGKILL);         // Send kill signal to child
            waitpid(pid, NULL, 0);      // Wait to reap child (prevent zombie)
            if (verbose)
                printf("Bad function: timed out after %u seconds\n", timeout);
            return (0);                 // Return 0 = bad function
        }
        return (-1);    // Some other error occurred
    }
    
    if (WIFEXITED(status))
    {
        if (WEXITSTATUS(status) == 0)
        {
            if (verbose)
                printf("Nice function!\n");
            return (1);     // Return 1 = nice function
        }
        else
        {
            if (verbose)
                printf("Bad function: exited with code %d\n", WEXITSTATUS(status));
            return (0);     // Return 0 = bad function
        }
    }
    if (WIFSIGNALED(status))
    {
        if (verbose)
            printf("Bad function: %s\n", strsignal(WTERMSIG(status)));
        return (0);         // Return 0 = bad function
    }
    return (-1);
}

void    good_function(void)
{
    printf("Good function is running!\n");
}

void    bad_function(void)
{
    printf("Bad function is running!\n");
    exit(42);
}

void    infinite_function(void)
{
    printf("Infinite function is running!\n");
    while (1)
        ;
}

int main(void)
{
    printf("=== Test 1: Good function ===\n");
    printf("Result: %d\n\n", sandbox(good_function, 3, true));

    printf("=== Test 2: Bad function ===\n");
    printf("Result: %d\n\n", sandbox(bad_function, 3, true));

    printf("=== Test 3: Timeout ===\n");
    printf("Result: %d\n\n", sandbox(infinite_function, 2, true));

    return (0);
}