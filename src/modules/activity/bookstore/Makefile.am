include ./config.mak

MAINTAINERCLEANFILES = Makefile.in

MODULE = bookstore

# data files for the module
filesdir = $(libdir)/enna/modules/

# the module .so file
INCLUDES = \
-I. \
-I$(top_srcdir) \
-I$(top_srcdir)/src/modules/$(MODULE) \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/modules \
@ENNA_CFLAGS@

EXTRA_DIST= \
bookstore.h \
bookstore_gocomics.h

if BUILD_ACTIVITY_BOOKSTORE

pkgdir = $(libdir)/enna/modules

pkg_LTLIBRARIES = activity_bookstore.la

activity_bookstore_la_SOURCES = $(SRCS_ACTIVITY_BOOKSTORE)

activity_bookstore_la_LIBADD = @ENNA_LIBS@
activity_bookstore_la_LDFLAGS = -module -avoid-version
activity_bookstore_la_DEPENDENCIES = $(top_builddir)/config.h

uninstall:
	rm -rf $(DESTDIR)$(libdir)/enna/modules/activity_$(MODULE).*

endif
