CC = m68k-atari-mint-gcc
AS = m68k-atari-mint-as -traditional

CFLAGS = -O2 -fomit-frame-pointer

SRCDIR = src
OBJDIR = obj
BUILDDIR = build

FULL_OBJS = $(OBJDIR)/main_full.o $(OBJDIR)/demo.o $(OBJDIR)/overscan_ste_full.o
VERTICAL_OBJS = $(OBJDIR)/main_vertical.o $(OBJDIR)/demo.o $(OBJDIR)/overscan_ste_vertical.o

all: $(BUILDDIR)/FULL.TOS $(BUILDDIR)/VERTICAL.TOS $(BUILDDIR)/NONE.TOS

$(BUILDDIR)/FULL.TOS: $(FULL_OBJS) | $(BUILDDIR)
	$(CC) -o $@ $^

$(BUILDDIR)/VERTICAL.TOS: $(VERTICAL_OBJS) | $(BUILDDIR)
	$(CC) -o $@ $^

NONE_OBJS = $(OBJDIR)/main_none.o $(OBJDIR)/demo.o

$(BUILDDIR)/NONE.TOS: $(NONE_OBJS) | $(BUILDDIR)
	$(CC) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.s | $(OBJDIR)
	$(AS) -o $@ $<

$(OBJDIR):
	mkdir -p $@

$(BUILDDIR):
	mkdir -p $@

clean:
	rm -f $(OBJDIR)/*.o $(BUILDDIR)/*.TOS
