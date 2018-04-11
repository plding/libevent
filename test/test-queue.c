#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct event {
    TAILQ_ENTRY (event) ev_next;
    int ev_fd;
};

TAILQ_HEAD (event_list, event);

struct event_list eventqueue;

static void
event_set(struct event *ev, int fd)
{
    ev->ev_fd = fd;

    printf("ev(%p)\n", ev);
    printf("  ev_next(%p)\n", &ev->ev_next);
    printf("    tqe_next(%p): %p\n", &ev->ev_next.tqe_next, ev->ev_next.tqe_next);
    printf("    tqe_prev(%p): %p\n", &ev->ev_next.tqe_prev, ev->ev_next.tqe_prev);
    printf("  ev_fd(%p): %d\n\n", &ev->ev_fd, ev->ev_fd);
}

static void
event_add(struct event *ev)
{
    TAILQ_INSERT_TAIL(&eventqueue, ev, ev_next);

    printf("---------------------- after insert %p ----------------------\n", ev);

    printf("eventqueue(%p)\n", &eventqueue);
    printf("  tqh_first(%p): %p\n", &eventqueue.tqh_first, eventqueue.tqh_first);
    printf("  tqh_last(%p) : %p\n", &eventqueue.tqh_last, eventqueue.tqh_last);
    printf("\n");

    printf("ev(%p)\n", ev);
    printf("  ev_next(%p)\n", &ev->ev_next);
    printf("    tqe_next(%p): %p\n", &ev->ev_next.tqe_next, ev->ev_next.tqe_next);
    printf("    tqe_prev(%p): %p\n", &ev->ev_next.tqe_prev, ev->ev_next.tqe_prev);
    printf("  ev_fd(%p): %d\n\n", &ev->ev_fd, ev->ev_fd);
}

int
main(void)
{
    struct event ev1, ev2, ev3;

    printf("---------------------- initialize ----------------------\n");

    TAILQ_INIT(&eventqueue);
    printf("eventqueue(%p)\n", &eventqueue);
    printf("  tqh_first(%p): %p\n", &eventqueue.tqh_first, eventqueue.tqh_first);
    printf("  tqh_last(%p) : %p\n", &eventqueue.tqh_last, eventqueue.tqh_last);
    printf("\n");

    event_set(&ev1, 1);
    event_set(&ev2, 2);
    event_set(&ev3, 3);

    event_add(&ev1);
    event_add(&ev2);
    event_add(&ev3);

    return 0;
}
