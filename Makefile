.PHONY: all clean

all: hardshutdown

hardshutdown: hardshutdown.c
	$(CC) -Wall -Os -o $@ $<

clean:
	rm -vf hardshutdown

install: hardshutdown
	mkdir -p "$(PREFIX)/bin"
	cp -v hardshutdown "$(PREFIX)/bin/"
	( \
	cd "$(PREFIX)/bin"; \
	ln -s hardshutdown reboot; \
	ln -s hardshutdown poweroff; \
	)
