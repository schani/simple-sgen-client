all : test-sgen

SOURCES = \
	mono/mono/metadata/sgen-gc.c		\
	mono/mono/metadata/sgen-alloc.c	\
	mono/mono/metadata/sgen-nursery-allocator.c	\
	mono/mono/metadata/sgen-simple-nursery.c	\
	mono/mono/metadata/sgen-split-nursery.c	\
	mono/mono/metadata/sgen-marksweep.c	\
	mono/mono/metadata/sgen-los.c	\
	mono/mono/metadata/sgen-cardtable.c	\
	mono/mono/metadata/sgen-descriptor.c	\
	mono/mono/metadata/sgen-fin-weak-hash.c	\
	mono/mono/metadata/sgen-gray.c	\
	mono/mono/metadata/sgen-pointer-queue.c	\
	mono/mono/metadata/sgen-internal.c	\
	mono/mono/metadata/sgen-hash-table.c	\
	mono/mono/metadata/sgen-pinning.c	\
	mono/mono/metadata/sgen-pinning-stats.c	\
	mono/mono/metadata/sgen-protocol.c	\
	mono/mono/metadata/sgen-workers.c	\
	mono/mono/metadata/sgen-memory-governor.c	\
	mono/mono/metadata/sgen-debug.c	\
	mono/mono/metadata/gc-parse.c	\
	mono/mono/metadata/gc-memfuncs.c	\
	mono/mono/metadata/gc-stats.c	\
	mono/mono/utils/lock-free-queue.c	\
	mono/mono/utils/lock-free-alloc.c	\
	simple-client.c			\
	test-sgen.c

test-sgen : $(SOURCES)
	gcc -o test-sgen -Wall -DHAVE_SGEN_GC -DSGEN_CLIENT_HEADER=\"simple-client.h\" -DSGEN_WITHOUT_MONO -O0 -g -I. -I./mono/ $(SOURCES) `pkg-config --cflags --libs glib-2.0` -lpthread -lm
