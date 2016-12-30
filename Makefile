prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
DESTDIR =
INSTALL = /usr/bin/install
RM = /bin/rm

install:
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) bin/xlsx2tsv $(DESTDIR)$(bindir)/xlsx2tsv

uninstall:
	$(RM) $(DESTDIR)$(bindir)/xlsx2tsv

