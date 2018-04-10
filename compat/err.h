#ifndef _ERR_H_
#define _ERR_H_

void    err(int eval, const char *fmt, ...);
void    warn(const char *fmt, ...);
void    errx(int eval, const char *fmt, ...);
void    warnx(const char *fmt, ...);

#endif /* _ERR_H_ */
