bin_PROGRAMS = cdpfglserver

DEFS = -I../libcdpfgl $(GLIB_CFLAGS) $(GIO_CFLAGS)       \
	    $(JANSSON_CFLAGS) $(MHD_CFLAGS)

cdpfglserver_LDFLAGS = $(LDFLAGS) -lm
cdpfglserver_LDADD = $(GLIB_LIBS) $(GIO_LIBS)  -L../libcdpfgl -lcdpfgl \
		     $(JANSSON_LIBS) $(MHD_LIBS)

cdpfglserver_HEADERFILES =  server.h        \
							options.h       \
							backend.h       \
							file_backend.h

cdpfglserver_SOURCES =  server.c                    \
						options.c                   \
						backend.c                   \
						file_backend.c              \
						$(cdpfglserver_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS) $(JANSSON_CFLAGS) $(MHD_CFLAGS)
