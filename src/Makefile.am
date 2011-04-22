bin_PROGRAMS = mulk

INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include

mulk_SOURCES = main.c
# link against static lib
mulk_CFLAGS = -DMULK_STATIC_LIB
if ENABLE_NLS
mulk_CFLAGS += -DLOCALEDIR=\"$(localedir)\" 
endif
mulk_LDADD = -lmulk
mulk_LDFLAGS = -L$(top_builddir)/lib 

# link against static lib
mulk_LDFLAGS += -static

