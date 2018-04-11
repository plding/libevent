#ifndef _EVENT_H_
#define _EVENT_H_

#include <sys/queue.h>

#define EVLIST_TIMEOUT  0x01
#define EVLIST_INSERTED 0x02
#define EVLIST_SIGNAL   0x04
#define EVLIST_ACTIVE   0x08
#define EVLIST_INIT     0x80

#define EV_TIMEOUT      0x01
#define EV_READ         0x02
#define EV_WRITE        0x04
#define EV_SIGNAL       0x08
#define EV_PERSIST      0x10    /* Persistant event */

struct event {
    TAILQ_ENTRY (event) ev_next;

    int ev_fd;
    short ev_events;
    short ev_ncalls;
    short *ev_pncalls;  /* Allows deletes in callback */

    struct timeval ev_timeout;

    void (*ev_callback)(int, short, void *);
    void *ev_arg;

    int ev_res;     /* result passed to event callback */
    int ev_flags;
};

TAILQ_HEAD(event_list, event);

struct eventop {
    char *name;
    void *(*init)(void);
    int (*add)(void *, struct event *);
};

void event_init(void);

void event_set(struct event *, int, short, void (*)(int, short, void *), void *);
int event_add(struct event *, struct timeval *);

#endif /* _EVENT_H_ */
