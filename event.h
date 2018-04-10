#ifndef _EVENT_H_
#define _EVENT_H_

struct event {

};

struct eventop {
    char *name;
    void *(*init)(void);
};

void event_init(void);

#endif /* _EVENT_H_ */
