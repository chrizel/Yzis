#bin_PROGRAMS=yzis
#yzis_SOURCES=yzis.c xmalloc.c system.h aclocal.m4 yzis-opt.c yzis-opt.h

#  add " -lm" to the following if yzis requires the math library
#yzis_LDADD=-lopts

#EXTRA_DIST=yzis.lsm.in yzis.spec.in yzis-opt.def yzis.1 manpage.tpl

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk

#yzis.o: yzis-opt.h
#yzis-opt.o: yzis-opt.c
#yzis-opt.c yzis-opt.h: $(srcdir)/yzis-opt.def
#	autogen -L/usr/share/autogen $(srcdir)/yzis-opt.def

#yzis.1: $(srcdir)/yzis-opt.def
#	autogen -T manpage.tpl  $(srcdir)/yzis-opt.def
#	awk -f updater yzis.1 > tmp$$$$ && mv tmp$$$$ yzis.1 rm updater

# info_TEXINFOS = yzis.texinfo
# yzis_TEXINFOS = gpl.texinfo

#  install the man page
#man_MANS=yzis.1
SUBDIRS=libyzis cyzis
