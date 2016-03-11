CC=cc
LD=$(CC)

LIBS=`pkg-config fuse --libs` `pkg-config xcb --libs` `pkg-config xcb-icccm --libs`
CFLAGS=-Wall -std=gnu11 -c `pkg-config fuse --cflags` `pkg-config xcb --cflags`

PREFIX?=/usr

SRCDIR=src
OBJDIR=obj
BINDIR=bin

SRC := $(wildcard $(SRCDIR)/*.c)
OBJ := $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: x11fs

$(OBJDIR):
	mkdir $(OBJDIR)

$(BINDIR):
	mkdir $(BINDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(wildcard $(SRCDIR)/*.h) $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ $<

x11fs: $(BINDIR) $(OBJ)
	$(LD) $(OBJ) $(LIBS) -o $(BINDIR)/$@

install: x11fs
	mkdir -p $(DESTDIR)$(PREFIX)/bin/
	cp $(BINDIR)/x11fs $(DESTDIR)$(PREFIX)/bin/

uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/x11fs

clean:
	rm $(OBJDIR)/*.o
	rm $(BINDIR)/x11fs
