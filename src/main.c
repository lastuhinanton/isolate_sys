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
