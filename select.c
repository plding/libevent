#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <sys/_time.h>
#endif
#include <sys/queue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#include "event.h"

struct selectop {
    int event_fds;      /* Highest fd in fd set */
    int event_fdsz;
    fd_set *event_readset;
    fd_set *event_writeset;
} sop;

void *select_init(void);
int select_add(void *, struct event *);

const struct eventop selectops = {
    "select",
    select_init,
    select_add
};

void *
select_init(void)
{
    /* Disable kqueue when this environment variable is set */
    if (getenv("EVENT_NOSELECT"))
        return NULL;

    memset(&sop, 0, sizeof(sop));

    return &sop;
}

int
select_add(void *arg, struct event *ev)
{
    struct selectop *sop = arg;

    /*
     * Keep track of the highest fd, so that we can calculate the size
     * of the fd_sets for select(2)
     */
    if (sop->event_fds < ev->ev_fd)
        sop->event_fds = ev->ev_fd;

    return 0;
}
