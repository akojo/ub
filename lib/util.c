#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static char buf[BUFSIZ];

void error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	perror(buf);
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

