.PHONY: blitter hardware-scrolling all clean

all: blitter hardware-scrolling

blitter:
	$(MAKE) -C blitter

hardware-scrolling:
	$(MAKE) -C hardware-scrolling

clean:
	$(MAKE) -C blitter clean
	$(MAKE) -C hardware-scrolling clean
