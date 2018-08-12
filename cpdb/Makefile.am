include $(top_srcdir)/Makefile.decl

backenddir = $(libdir)/gtk-3.0/$(GTK_BINARY_VERSION)/printbackends

backend_LTLIBRARIES = libprintbackend-cpdb.la

libprintbackend_cpdb_la_SOURCES = \
	gtkprintbackendcpdb.c

noinst_HEADERS = \
	gtkprintbackendcpdb.h

libprintbackend_cpdb_la_CPPFLAGS = \
	-I$(top_srcdir)													\
	-I$(top_srcdir)/gtk											\
	-I$(top_builddir)/gtk 									\
	-I$(top_srcdir)/gdk    									\
	-I$(top_builddir)/gdk										\
	$(CPDBACKEND_CFLAGS)
	-DGTK_COMPILATION												\
	-DGTK_PRINT_BACKEND_ENABLE_UNSUPPORTED	\
	$(AM_CPPFLAGS)

libprintbackend_cpdb_la_CFLAGS = \
	$(GTK_DEP_CFLAGS)			\
	$(GTK_DEBUG_FLAGS)		\
	$(AM_CFLAGS)

libprintbackend_cpdb_la_LDFLAGS = \
	-avoid-version -module $(no_undefined)

libprintbackend_cpdb_la_LIBADD = \
	$(top_builddir)/gtk/libgtk-3.la \
	$(CPDBACKEND_LIBS)
	$(GTK_DEP_LIBS)

-include $(top_srcdir)/git.mk
