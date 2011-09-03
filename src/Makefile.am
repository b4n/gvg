
NULL =

# FIXME:
plugindir = $(libdir)

plugin_LTLIBRARIES  = libgvg.la

libgvg_la_CFLAGS    = $(GVG_CFLAGS) -DG_LOG_DOMAIN=\"GVG\"
libgvg_la_LIBADD    = $(GVG_LIBS)
libgvg_la_LDFLAGS   = 
libgvg_la_SOURCES   = gvg-plugin.c \
                      gvg-plugin.h \
                      gvg-args-builder.h \
                      gvg.c \
                      gvg.h \
                      gvg-memcheck.c \
                      gvg-memcheck.h \
                      gvg-xml-parser.c \
                      gvg-xml-parser.h \
                      $(NULL)


check_PROGRAMS      = gvg-test

gvg_test_CFLAGS     = $(GVG_CFLAGS)
gvg_test_LDADD      = $(GVG_LIBS) libgvg.la
gvg_test_SOURCES    = gvg-test.c


check-local:
	@echo "CHECK"; \
	./gvg-test "df"; \
	#libtool execute gdb -ex run --args ./gvg-test "df"
