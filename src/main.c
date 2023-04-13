#define     _GNU_SOURCE
#include    <stdio.h>
#include    <sched.h>
#include    <stdlib.h>
#include    <stdarg.h>
#include    <unistd.h>
#include    <sys/prctl.h>
#include    <wait.h>
#include    <memory.h>

static void die(const char *fmt, ...)
{
    va_list params;

    va_start(params, fmt);
    vfprintf(stderr, fmt, params);
    va_end(params);
    exit(1);
}

struct params
{
    int     fd[2];
    char    **argv;
};

static void parse_args(int argc, char **argv, struct params *params)
{
    #define NEXT_ARG() do { argc--; argv++; } while (0)
    NEXT_ARG();

    if (argc < 1) {
        printf("Nothing to do!\n");
        exit(0);
    }

    params->argv = argv;
    #undef NEXT_ARG
}

#define STACKSIZE (1024*1024)
static char cmd_stack[STACKSIZE];

void await_setup(int pipe)
{
    char buf[2];
    if (read(pipe, buf, 2) != 2)
        die("Failed to read from pipe: %m\n");
}

static int cmd_exec(void *arg)
{
    if (prctl(PR_SET_PDEATHSIG, SIGKILL))
        die("cannot PR_SET_PDEATHSIG for chil process: %m\n");

    struct params *params = (struct params*) arg;
    await_setup(params->fd[0]);

    if (setgid(0) == -1)
        die("Failed to setgid: %m\n");
    if (setuid(0) == -1)
        die("Failed to setuid: %m\n");

    char **argv = params->argv;
    char *cmd = argv[0];
    printf("===========%s============\n", cmd);

    if (execvp(cmd, argv) == -1)
        die("Failed to exec %s: %m\n", cmd);
    
    die("¯\\_(ツ)_/¯");
    return 1;
}

static void write_file(char path[100], char line[100])
{
    FILE *f = fopen(path, "w");
    
    if (f == NULL)
        die("Failed to open file %s: %m\n", path);
    
    if (fwrite(line, 1, strlen(line), f) < 0)
        die("Failed to write to file %s:\n", path);

    if (fclose(f) != 0)
        die("Failed to close file %s: %m\n", path);
}

int main(int argc, char **argv)
{
    struct params params;
    memset(&params, 0, sizeof(struct params));

    parse_args(argc, argv, &params);

    if (pipe(params.fd) < 0)
        die("Failed to create pipe: %m");
    
    int clone_flags = SIGCHLD | CLONE_NEWUTS | CLONE_NEWUSER;
    int cmd_pid = clone(cmd_exec, cmd_stack + STACKSIZE, clone_flags, &params);

    if (cmd_pid < 0)
        die("Failed to clone: %m\n");

    int pipe = params.fd[1];

    prepare_userns(cmd_pid);

    if (write(pipe, "OK", 2) != 2)
        die("Failed to write to pipe: %m");
    if (close(pipe))
        die("Failed to close pipe: %m");
    
    if (waitpid(cmd_pid, NULL, 0) == -1)
        die("Failedd to wait pid %d: %m\n", cmd_pid);
    
    return 0;
}
