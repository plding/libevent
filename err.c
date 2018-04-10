#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

void
err(int eval, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if (fmt != NULL) {
        (void) vfprintf(stderr, fmt, ap);
        (void) fprintf(stderr, ": ");
    }
    va_end(ap);
    (void) fprintf(stderr, "%s\n", strerror(errno));
    exit(eval);
}

void
errx(int eval, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if (fmt != NULL)
        (void) vfprintf(stderr, fmt, ap);
    (void) fprintf(stderr, "\n");
    va_end(ap);
    exit(eval);
}

void
warn(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if (fmt != NULL) {
        (void) vfprintf(stderr, fmt, ap);
        (void) fprintf(stderr, ": ");
    }
    va_end(ap);
    (void) fprintf(stderr, "%s\n", strerror(errno));
}

void
warnx(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if (fmt != NULL) {
        (void) vfprintf(stderr, fmt, ap);
        (void) fprintf(stderr, "\n");
    }
    va_end(ap);
}
