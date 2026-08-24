# Copyright (C) 2026 Neil Rackett
# SPDX-License-Identifier: GPL-2.0-or-later

CC = m68k-atari-mint-gcc
AS = m68k-atari-mint-as -traditional

CFLAGS = -O2 -fomit-frame-pointer

SRCDIR = src
OBJDIR = obj
BUILDDIR = build

BOTTOM_OBJS = $(OBJDIR)/main_bottom.o $(OBJDIR)/demo.o $(OBJDIR)/overscan_bottom.o
TOP_OBJS = $(OBJDIR)/main_top.o $(OBJDIR)/demo.o $(OBJDIR)/overscan_top.o
BOTH_OBJS = $(OBJDIR)/main_both.o $(OBJDIR)/demo.o $(OBJDIR)/overscan_both.o

all: $(BUILDDIR)/TOP.TOS $(BUILDDIR)/BOTTOM.TOS $(BUILDDIR)/BOTH.TOS

$(BUILDDIR)/TOP.TOS: $(TOP_OBJS) | $(BUILDDIR)
	$(CC) -o $@ $^

$(BUILDDIR)/BOTTOM.TOS: $(BOTTOM_OBJS) | $(BUILDDIR)
	$(CC) -o $@ $^

$(BUILDDIR)/BOTH.TOS: $(BOTH_OBJS) | $(BUILDDIR)
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
