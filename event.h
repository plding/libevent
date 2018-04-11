#ifndef _EVENT_H_
#define _EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

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
    TAILQ_ENTRY (event) ev_active_next;

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
    int (*del)(void *, struct event *);
    int (*recalc)(void *, int);
    int (*dispatch)(void *, struct timeval *);
};

void event_init(void);
int event_dispatch(void);

int event_loop(int);

void event_set(struct event *, int, short, void (*)(int, short, void *), void *);
int event_add(struct event *, struct timeval *);
int event_del(struct event *);
void event_active(struct event *, int, short);

#ifdef __cplusplus
}
#endif

#endif /* _EVENT_H_ */
