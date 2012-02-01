bin_PROGRAMS = mulk

INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include

mulk_SOURCES = main.c
mulk_CFLAGS = 
if ENABLE_NLS
mulk_CFLAGS += -DLOCALEDIR=\"$(localedir)\" 
endif

mulk_LDFLAGS = 
mulk_LDADD = $(top_builddir)/lib/libmulk.la 

if ENABLE_STATIC
mulk_CFLAGS += -DMULK_STATIC_LIB
mulk_LDFLAGS += -static
endif

