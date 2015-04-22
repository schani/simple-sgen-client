all : test-sgen

MONO_DIR = mono

SOURCES = \
	$(MONO_DIR)/mono/metadata/sgen-gc.c		\
	$(MONO_DIR)/mono/metadata/sgen-alloc.c	\
	$(MONO_DIR)/mono/metadata/sgen-nursery-allocator.c	\
	$(MONO_DIR)/mono/metadata/sgen-simple-nursery.c	\
	$(MONO_DIR)/mono/metadata/sgen-split-nursery.c	\
	$(MONO_DIR)/mono/metadata/sgen-marksweep.c	\
	$(MONO_DIR)/mono/metadata/sgen-los.c	\
	$(MONO_DIR)/mono/metadata/sgen-cardtable.c	\
	$(MONO_DIR)/mono/metadata/sgen-descriptor.c	\
	$(MONO_DIR)/mono/metadata/sgen-fin-weak-hash.c	\
	$(MONO_DIR)/mono/metadata/sgen-gray.c	\
	$(MONO_DIR)/mono/metadata/sgen-pointer-queue.c	\
	$(MONO_DIR)/mono/metadata/sgen-internal.c	\
	$(MONO_DIR)/mono/metadata/sgen-hash-table.c	\
	$(MONO_DIR)/mono/metadata/sgen-pinning.c	\
	$(MONO_DIR)/mono/metadata/sgen-pinning-stats.c	\
	$(MONO_DIR)/mono/metadata/sgen-protocol.c	\
	$(MONO_DIR)/mono/metadata/sgen-workers.c	\
	$(MONO_DIR)/mono/metadata/sgen-thread-pool.c	\
	$(MONO_DIR)/mono/metadata/sgen-memory-governor.c	\
	$(MONO_DIR)/mono/metadata/sgen-debug.c	\
	$(MONO_DIR)/mono/metadata/gc-parse.c	\
	$(MONO_DIR)/mono/metadata/gc-memfuncs.c	\
	$(MONO_DIR)/mono/metadata/gc-stats.c	\
	$(MONO_DIR)/mono/utils/mono-mutex.c	\
	$(MONO_DIR)/mono/utils/monobitset.c	\
	$(MONO_DIR)/mono/utils/lock-free-queue.c	\
	$(MONO_DIR)/mono/utils/lock-free-alloc.c	\
	$(MONO_DIR)/mono/utils/lock-free-array-queue.c	\
	$(MONO_DIR)/mono/utils/hazard-pointer.c	\
	simple-client.c			\
	test-sgen.c


test-sgen : Makefile $(SOURCES)
	gcc -std=gnu99 -o test-sgen -Wall -Wno-attributes -DHAVE_SGEN_GC -DSGEN_CLIENT_HEADER=\"simple-client.h\" -DSGEN_WITHOUT_MONO -O0 -g -I. -I./$(MONO_DIR)/ $(SOURCES) `pkg-config --cflags --libs glib-2.0` -lpthread -lm
