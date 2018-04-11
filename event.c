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
void event_queue_remove(struct event *, int);
void event_process_active(void);

static struct event_list activequeue;
struct event_list eventqueue;

void
event_init(void)
{
    int i;

    TAILQ_INIT(&activequeue);
    TAILQ_INIT(&eventqueue);

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
event_process_active(void)
{
    struct event *ev;
    short ncalls;

    for (ev = TAILQ_FIRST(&activequeue); ev;
        ev = TAILQ_FIRST(&activequeue)) {
        event_queue_remove(ev, EVLIST_ACTIVE);

        /* Allows deletes to work */
        ncalls = ev->ev_ncalls;
        ev->ev_pncalls = &ncalls;
        while (ncalls) {
            ncalls--;
            ev->ev_ncalls = ncalls;

            fprintf(stderr, "%s: execute callback: %p\n", __func__, ev->ev_callback);
            (*ev->ev_callback)(ev->ev_fd, ev->ev_res, ev->ev_arg);
        }
    }
}

int
event_dispatch(void)
{
    return event_loop(0);
}

int
event_loop(int flags)
{
    int res, done;

    /* Calculate the initial events that we are waiting for */
    if (evsel->recalc(evbase, 0) == -1)
        return -1;

    done = 0;
    while (!done) {

        res = evsel->dispatch(evbase, NULL);

        if (res == -1)
            return -1;

        if (TAILQ_FIRST(&activequeue)) {
            event_process_active();
        }

        if (evsel->recalc(evbase, 0) == -1)
            return -1;
    }

    return 0;
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

int
event_del(struct event *ev)
{
    fprintf(stderr, "event_del: %p, callback %p\n",
        ev, ev->ev_callback);

    /* See if we are just active executing this event in a loop */
    if (ev->ev_ncalls && ev->ev_pncalls) {
        /* Abort loop */
        *ev->ev_pncalls = 0;
    }

    if (ev->ev_flags & EVLIST_ACTIVE)
        event_queue_remove(ev, EVLIST_ACTIVE);
    fprintf(stderr, "flags: %x, activequeue: %p\n", ev->ev_flags, TAILQ_FIRST(&activequeue));

    if (ev->ev_flags & EVLIST_INSERTED) {
        event_queue_remove(ev, EVLIST_INSERTED);
        fprintf(stderr, "flags: %x, eventqueue: %p\n", ev->ev_flags, TAILQ_FIRST(&eventqueue));
        return evsel->del(evbase, ev);
    }

    return 0;
}

void
event_active(struct event *ev, int res, short ncalls)
{
    /* We get different kinds of events, add them together */
    if (ev->ev_flags & EVLIST_ACTIVE) {
        ev->ev_res |= res;
        return;
    }

    ev->ev_res = res;
    ev->ev_ncalls = ncalls;
    ev->ev_pncalls = NULL;
    event_queue_insert(ev, EVLIST_ACTIVE);

    printf("%s: ev_res: %x, ev_ncalls: %d, ev_flags: %x\n",
        __func__, ev->ev_res, ev->ev_ncalls, ev->ev_flags);
}

void
event_queue_remove(struct event *ev, int queue)
{
    if (!(ev->ev_flags & queue))
        errx(1, "%s: %p(fd %d) not on queue %x", __func__,
            ev, ev->ev_fd, queue);

    ev->ev_flags &= ~queue;
    switch (queue) {
    case EVLIST_ACTIVE:
        TAILQ_REMOVE(&activequeue, ev, ev_active_next);
        break;
    case EVLIST_INSERTED:
        TAILQ_REMOVE(&eventqueue, ev, ev_next);
        break;
    default:
        errx(1, "%s: unknown queue %x", __func__, queue);
    }
}

void
event_queue_insert(struct event *ev, int queue)
{
    if (ev->ev_flags & queue)
        errx(1, "%s: %p(fd %d) already on queue %x", __func__,
            ev, ev->ev_fd, queue);

    ev->ev_flags |= queue;
    switch (queue) {
    case EVLIST_ACTIVE:
        TAILQ_INSERT_TAIL(&activequeue, ev, ev_active_next);
        break;
    case EVLIST_INSERTED:
        TAILQ_INSERT_TAIL(&eventqueue, ev, ev_next);
        break;
    default:
        errx(1, "%s: unknown queue %x", __func__, queue);
    }
}
