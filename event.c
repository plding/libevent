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

/* Prototypes */
void event_queue_insert(struct event *, int);

struct event_list eventqueue;

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

    // if (getenv("EVENT_SHOW_METHOD"))
        fprintf(stderr, "libevent using: %s\n", evsel->name);
}

void
event_set(struct event *ev, int fd, short events,
    void (*callback)(int, short, void *), void *arg)
{
    ev->ev_callback = callback;
    ev->ev_arg = arg;
    ev->ev_fd = fd;
    ev->ev_events = events;
    ev->ev_flags = EVLIST_INIT;
    ev->ev_ncalls = 0;
    ev->ev_pncalls = NULL;
}

int
event_add(struct event *ev, struct timeval *tv)
{
    fprintf(stderr, "event_add: event: %p, %s%s%scall %p\n",
        ev,
        ev->ev_events & EV_READ ? "EV_READ " : " ",
        ev->ev_events & EV_WRITE ? "EV_WRITE " : " ",
        tv ? "EV_TIMEOU " : " ",
        ev->ev_callback);

    // assert(!(ev->ev_flags & ~EVLIST_ALL));

    if ((ev->ev_events & (EV_READ|EV_WRITE)) &&
        !(ev->ev_flags & (EVLIST_INSERTED|EVLIST_ACTIVE))) {
        event_queue_insert(ev, EVLIST_INSERTED);

        return (evsel->add(evbase, ev));
    }

    return 0;
}

void
event_queue_insert(struct event *ev, int queue)
{
    if (ev->ev_flags & queue)
        errx(1, "%s: %p(fd %d) already on queue %x", __func__,
            ev, ev->ev_fd, queue);

    ev->ev_flags |= queue;
    switch (queue) {
    case EVLIST_INSERTED:
        TAILQ_INSERT_TAIL(&eventqueue, ev, ev_next);
        break;
    default:
        errx(1, "%s: unknown queue %x", __func__, queue);
    }
}
