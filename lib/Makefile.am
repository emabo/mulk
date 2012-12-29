if ENABLE_METALINK
  METALINK_SOURCES=chunk_list.c chunk_list.h metalink_list.c metalink_list.h country_codes.c country_codes.h
if ENABLE_CHECKSUM
  CHECKSUM_SOURCES=checksum.c checksum.h 
endif
endif

INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include

lib_LTLIBRARIES=libmulk.la
libmulk_la_SOURCES= \
  defines.h \
  m_malloc.c m_malloc.h \
  buffer_array.c buffer_array.h \
  parse.c parse.h \
  option_obj.c option_obj.h \
  string_obj.c string_obj.h \
  gif_obj.c gif_obj.h \
  png_obj.c png_obj.h \
  jpg_obj.c jpg_obj.h \
  url_list.c url_list.h \
  file_obj.c file_obj.h \
  mime_types.c mime_types.h \
  uri_parser.c uri_parser.h \
  mulk.c \
  $(METALINK_SOURCES) \
  $(CHECKSUM_SOURCES) \
  gettext.h

libmulk_la_CFLAGS=$(CFLAG_VISIBILITY) -DBUILDING_MULK_LIB
if ENABLE_NLS
libmulk_la_CFLAGS+=-DLOCALEDIR=\"$(localedir)\"
endif
libmulk_la_LDFLAGS=-no-undefined -version-info $(SHARED_VERSION_INFO)

noinst_HEADERS= \
  m_malloc.h gettext.h buffer_array.h defines.h file_obj.h gif_obj.h \
  png_obj.h jpg_obj.h parse.h url_list.h \
  mime_types.h chunk_list.h metalink_list.h uri_parser.h \
  checksum.h option_obj.h string_obj.h country_codes.h

