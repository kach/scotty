PREFIX=/usr/local
MANDIR=/usr/local/share/man

README : scotty.1
	groff -man -Tascii scotty.1 | col -bx > README.txt

scotty : scotty.c
	gcc scotty.c -o scotty

install : scotty
	mkdir -p $(PREFIX)/bin
	mkdir -p $(MANDIR)/man1
	mv scotty $(PREFIX)/bin/scotty
	cp scotty.1 $(MANDIR)/man1/scotty.1
