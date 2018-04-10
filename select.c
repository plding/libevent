#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "event.h"

struct selectop {
    int event_fds;      /* Highest fd in fd set */
    int event_fdsz;
    fd_set *event_readset;
    fd_set *event_writeset;
} sop;

void *select_init(void);

const struct eventop selectops = {
    "select",
    select_init
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
