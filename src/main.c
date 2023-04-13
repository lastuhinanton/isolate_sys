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
}

static void parse_args(int argc, char **argv, struct params *params)
{
#define NEXT_ARG() do { argc--; argv++; } while (0);
    NEXT_ARG();

    if (argc < 1)
    {
        printf("Nothing to do!\n");
        exit(0);
    }
    params->argv = argv;
#undef  NEXT_ARG
}

#define STACKSIZE (1024*1024)
static char cmd_stack[STACKSIZE];

void await_setup(int pipe)
{
    char buf[2];
    if (read(pipe, buf, 2) != 2)
        die("Failed to read from pipe: %m\n");
}

