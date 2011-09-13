
null =

sources         = gvg-plugin.c \
                  gvg.c \
                  gvg-memcheck.c \
                  gvg-memcheck-parser.c \
                  gvg-memcheck-options.c \
                  gvg-memcheck-store.c \
                  gvg-options.c \
                  gvg-ui.c \
                  gvg-xml-parser.c \
                  $(null)
headers         = gvg-plugin.h \
                  gvg-args-builder.h \
                  gvg.h \
                  gvg-memcheck.h \
                  gvg-memcheck-parser.h \
                  gvg-memcheck-options.h \
                  gvg-memcheck-store.h \
                  gvg-options.h \
                  gvg-ui.h \
                  gvg-xml-parser.h \
                  $(null)
autogen_sources = gvg-enum-types.c
autogen_headers = gvg-enum-types.h

CLEANFILES      = $(autogen_sources) \
                  $(autogen_headers)

EXTRA_DIST      = gvg-enum-types.c.tpl \
                  gvg-enum-types.h.tpl \
                  $(null)

# FIXME:
plugindir = $(libdir)

plugin_LTLIBRARIES  = libgvg.la

libgvg_la_CFLAGS    = $(GVG_CFLAGS) -DG_LOG_DOMAIN=\"GVG\"
libgvg_la_LIBADD    = $(GVG_LIBS)
libgvg_la_LDFLAGS   = 
libgvg_la_SOURCES   = $(autogen_sources) \
                      $(autogen_headers) \
                      $(sources) \
                      $(headers) \
                      $(null)


check_PROGRAMS      = gvg-test

gvg_test_CFLAGS     = $(GVG_CFLAGS)
gvg_test_LDADD      = $(GVG_LIBS) libgvg.la
gvg_test_SOURCES    = gvg-test.c


gvg-enum-types.c: $(srcdir)/gvg-enum-types.c.tpl gvg-enum-types.h $(headers) Makefile
	$(AM_V_GEN)$(GLIB_MKENUMS) --template $< $(headers:%=$(srcdir)/%) > $@
gvg-enum-types.h: $(srcdir)/gvg-enum-types.h.tpl $(headers) Makefile
	$(AM_V_GEN)$(GLIB_MKENUMS) --template $< $(headers:%=$(srcdir)/%) > $@


check-local:
	@echo "CHECK"; \
	./gvg-test "df"; \
	#libtool execute gdb -ex run --args ./gvg-test "df"
