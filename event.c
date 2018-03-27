#include <stdlib.h>
#include <sys/queue.h>

#include "event.h"

TAILQ_HEAD(event_wlist, event) writequeue;
TAILQ_HEAD(event_rlist, event) readqueue;

void
event_init(void)
{
    TAILQ_INIT(&writequeue);
    TAILQ_INIT(&readqueue);
}
