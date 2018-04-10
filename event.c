#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/tree.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <sys/_time.h>
#endif
#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <err.h>
#include <assert.h>

#include "event.h"

#ifdef HAVE_SELECT
extern const struct eventop selectops;
#endif

/* In order of preference */
const struct eventop *eventops[] = {
#ifdef HAVE_SELECT
    &selectops,
#endif
    NULL
};

const struct eventop *evsel;
void *evbase;

void
event_init(void)
{
    int i;

    evbase = NULL;
    for (i = 0; eventops[i] && !evbase; i++) {
        evsel = eventops[i];

        evbase = evsel->init();
    }

    if (evbase == NULL)
        errx(1, "%s: no event mechanism available", __func__);

    if (getenv("EVENT_SHOW_METHOD"))
        fprintf(stderr, "libevent using: %s\n", evsel->name);
}
