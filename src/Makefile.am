SUBDIRS = TS ARIB

bin_PROGRAMS = tsparser eit2xml
eit2xml_SOURCES = eit2xml.cpp epg.cpp epg.h aribstr.cpp aribstr.h
eit2xml_LDADD = ARIB/libarib.a TS/libts.a

tsparser_SOURCES = tsparser.cpp aribstr.cpp aribstr.h
tsparser_LDADD = ARIB/libarib.a TS/libts.a
