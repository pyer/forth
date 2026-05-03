# Build pf
# Pyer's Forth
# Pierre BAZONNARD
.PHONY: clean compile manual test

CC = gcc
#ARCH = arm
ARCH = x86
CFLAGS = -D$(ARCH) -Isrc -O1 -g -Wall -Wstrict-prototypes -funsigned-char -fPIC
LFLAGS = -D$(ARCH) -g -fcommon  -Wall -Wstrict-prototypes -funsigned-char -fPIC -rdynamic -lm

includes = src/compiler.h src/config.h src/const.h src/exception.h src/floating.h src/history.h src/interpret.h src/macro.h src/terminal.h src/types.h src/version.h
objects = target/main.o target/compiler.o target/core.o target/exception.o target/facility.o target/interpret.o target/file.o target/floating.o target/history.o target/terminal.o target/shell.o target/signals.o target/tools.o

all: clean compile manual test

clean:
	rm -rf target
	find ./ -name "*~" -delete

compile: target $(objects)
	@echo "Architecture is ${ARCH}"
	$(CC) $(LFLAGS) -o pf $(objects)
ifeq ($(ARCH),arm)
	@echo "before compiling ARM architecture, set up environment: source /usr/local/angstrom/arm/environment-setup"
	scp pf          bbb:/usr/bin/pf
	scp lib/pf.4th  bbb:/usr/lib/pf.4th
else
	sudo install -m 755 pf /usr/bin/pf
	sudo install -m 755 lib/pf.4th  /usr/lib/pf.4th
endif

target:
	@mkdir -p target

target/%.o : src/%.c $(includes)
	$(CC) $(CFLAGS) -c -o $@ $<

manual:
	sudo install -m 644 doc/pf.txt /usr/share/man/pf.txt
	sudo install -m 644 doc/pf.1   /usr/share/man/man1/pf.1
	sudo gzip -f /usr/share/man/man1/pf.1

test:
	pf -f test/see.4th
	pf -f test/test.4th

