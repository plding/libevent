#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <event.h>

int
main(int argc, char **argv)
{
    struct event evfifo;
    struct stat st;
    char *fifo = "event.fifo";
    int socket;

    if (lstat(fifo, &st) == 0) {
        if ( (st.st_mode & S_IFMT) == S_IFREG) {
            errno = EEXIST;
            perror("lstat");
            exit(1);
        }
    }

    unlink(fifo);
    if (mkfifo(fifo, 0600) == -1) {
        perror("mkfifo");
        exit(1);
    }

    socket = open(fifo, O_RDONLY | O_NONBLOCK, 0);

    if (socket == -1) {
        perror("open");
        exit(1);
    }

    fprintf(stderr, "Write data to %s\n", fifo);

    event_init();

    return 0;
}
