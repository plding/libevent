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

extern struct event_list eventqueue;

#ifndef howmany
#define howmany(x, y) (((x)+((y)-1))/(y))
#endif

struct selectop {
    int event_fds;      /* Highest fd in fd set */
    int event_fdsz;
    fd_set *event_readset;
    fd_set *event_writeset;
} sop;

void *select_init(void);
int select_add(void *, struct event *);
int select_del(void *, struct event *);
int select_recalc(void *, int);
int select_dispatch(void *, struct timeval *);

const struct eventop selectops = {
    "select",
    select_init,
    select_add,
    select_del,
    select_recalc,
    select_dispatch
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

/*
 * Called with the highest fd that we known about. If it is 0, completely
 * recalculate everything.
 */

int
select_recalc(void *arg, int max)
{
    struct selectop *sop = arg;
    fd_set *readset, *writeset;
    struct event *ev;
    int fdsz;

    if (sop->event_fds < max)
        sop->event_fds = max;

    if (!sop->event_fds) {
        TAILQ_FOREACH(ev, &eventqueue, ev_next)
            if (ev->ev_fd > sop->event_fds)
                sop->event_fds = ev->ev_fd;
    }

    fdsz = howmany(sop->event_fds + 1, NFDBITS) * sizeof(fd_mask);

    if (fdsz > sop->event_fdsz) {
        if ((readset = realloc(sop->event_readset, fdsz)) == NULL) {
            return -1;
        }

        if ((writeset = realloc(sop->event_writeset, fdsz)) == NULL) {
            free(readset);
            return -1;
        }

        memset((char *) readset + sop->event_fdsz, 0,
            fdsz - sop->event_fdsz);
        memset((char *) writeset + sop->event_fdsz, 0,
            fdsz - sop->event_fdsz);

        sop->event_readset = readset;
        sop->event_writeset = writeset;
        sop->event_fdsz = fdsz;
    }

    return 0;
}

int
select_dispatch(void *arg, struct timeval *tv)
{
    int maxfd, res;
    struct event *ev, *next;
    struct selectop *sop = arg;

    memset(sop->event_readset, 0, sop->event_fdsz);
    memset(sop->event_writeset, 0, sop->event_fdsz);

    TAILQ_FOREACH(ev, &eventqueue, ev_next) {
        if (ev->ev_events & EV_WRITE)
            FD_SET(ev->ev_fd, sop->event_writeset);
        if (ev->ev_events & EV_READ)
            FD_SET(ev->ev_fd, sop->event_readset);
    }

    res = select(sop->event_fds + 1, sop->event_readset,
        sop->event_writeset, NULL, tv);

    if (res == -1) {
        if (errno != EINTR) {
            return -1;
        }

        return 0;
    }

    maxfd = 0;
    for (ev = TAILQ_FIRST(&eventqueue); ev != NULL; ev = next) {
        next = TAILQ_NEXT(ev, ev_next);

        res = 0;
        if (FD_ISSET(ev->ev_fd, sop->event_readset))
            res |= EV_READ;
        if (FD_ISSET(ev->ev_fd, sop->event_writeset))
            res |= EV_WRITE;
        res &= ev->ev_events;

        if (res) {
            if (!(ev->ev_events & EV_PERSIST))
                event_del(ev);
            event_active(ev, res, 1);
        } else if (ev->ev_fd > maxfd)
            maxfd = ev->ev_fd;
    }

    sop->event_fds = maxfd;

    return 0;
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

/*
 * Nothing to be done here.
 */

int
select_del(void *arg, struct event *ev)
{
    return 0;
}
