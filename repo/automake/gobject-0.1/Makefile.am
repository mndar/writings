AUTOMAKE_OPTIONS = foreign
AM_CFLAGS = "-Werror"
PKGLIBS = glib-2.0 gobject-2.0
bin_PROGRAMS = testclass

testclass_SOURCES = src/testclass.c src/myclass.c 
testclass_CFLAGS = ${GOBJECT_CFLAGS} ${GLIB_LDFLAGS}
testclass_LDADD = ${GOBJECT_LIBS} ${GLIB_LIBS}
