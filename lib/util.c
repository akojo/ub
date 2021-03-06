#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/errno.h>

static char buf[BUFSIZ];

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);

    if (errno)
        perror(buf);
    else
        fprintf(stderr, "%s\n", buf);

    exit(1);
}

void warning(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

void quit(char *msg)
{
    printf("%s\n", msg);
    exit(0);
}
