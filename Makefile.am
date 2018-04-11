AUTOMAKE_OPTIONS = foreign no-dependencies

CFLAGS = -Wall @CFLAGS@
SUBDIRS = . sample test

lib_LIBRARIES = libevent.a

libevent_a_SOURCES = event.c
libevent_a_LIBADD = @LIBOBJS@

include_HEADERS = event.h

INCLUDES = -Icompat

DISTCLEANFILES = *~
