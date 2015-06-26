TARGETS = test-sgen scheme

all : $(TARGETS)

clean :
	rm -f $(TARGETS)

MONO_DIR = mono

SGEN_SOURCES = \
	$(MONO_DIR)/mono/sgen/sgen-gc.c		\
	$(MONO_DIR)/mono/sgen/sgen-alloc.c	\
	$(MONO_DIR)/mono/sgen/sgen-nursery-allocator.c	\
	$(MONO_DIR)/mono/sgen/sgen-simple-nursery.c	\
	$(MONO_DIR)/mono/sgen/sgen-split-nursery.c	\
	$(MONO_DIR)/mono/sgen/sgen-marksweep.c	\
	$(MONO_DIR)/mono/sgen/sgen-los.c	\
	$(MONO_DIR)/mono/sgen/sgen-cardtable.c	\
	$(MONO_DIR)/mono/sgen/sgen-descriptor.c	\
	$(MONO_DIR)/mono/sgen/sgen-fin-weak-hash.c	\
	$(MONO_DIR)/mono/sgen/sgen-gray.c	\
	$(MONO_DIR)/mono/sgen/sgen-pointer-queue.c	\
	$(MONO_DIR)/mono/sgen/sgen-internal.c	\
	$(MONO_DIR)/mono/sgen/sgen-hash-table.c	\
	$(MONO_DIR)/mono/sgen/sgen-pinning.c	\
	$(MONO_DIR)/mono/sgen/sgen-pinning-stats.c	\
	$(MONO_DIR)/mono/sgen/sgen-protocol.c	\
	$(MONO_DIR)/mono/sgen/sgen-workers.c	\
	$(MONO_DIR)/mono/sgen/sgen-thread-pool.c	\
	$(MONO_DIR)/mono/sgen/sgen-memory-governor.c	\
	$(MONO_DIR)/mono/sgen/sgen-debug.c	\
	$(MONO_DIR)/mono/utils/parse.c	\
	$(MONO_DIR)/mono/utils/memfuncs.c	\
	$(MONO_DIR)/mono/metadata/gc-stats.c	\
	$(MONO_DIR)/mono/utils/mono-mutex.c	\
	$(MONO_DIR)/mono/utils/monobitset.c	\
	$(MONO_DIR)/mono/utils/lock-free-queue.c	\
	$(MONO_DIR)/mono/utils/lock-free-alloc.c	\
	$(MONO_DIR)/mono/utils/lock-free-array-queue.c	\
	$(MONO_DIR)/mono/utils/hazard-pointer.c	\
	client-common.c

TEST_SOURCES = \
	$(SGEN_SOURCES)			\
	simple-client.c			\
	test-sgen.c

SCHEME_SOURCES = \
	$(SGEN_SOURCES)			\
	scheme-client.c			\
	scheme.c

test-sgen : Makefile $(TEST_SOURCES) simple-client.h
	gcc -std=gnu99 -o test-sgen -Wall -Wno-attributes -DHAVE_SGEN_GC -DSGEN_CLIENT_HEADER=\"simple-client.h\" -DSGEN_WITHOUT_MONO -O0 -g -I. -I./$(MONO_DIR)/ $(TEST_SOURCES) `pkg-config --cflags --libs glib-2.0` -lpthread -lm

scheme : Makefile $(SCHEME_SOURCES) scheme-client.h
	gcc -std=gnu99 -o scheme -Wall -Wno-attributes -DHAVE_SGEN_GC -DSGEN_CLIENT_HEADER=\"scheme-client.h\" -DSGEN_WITHOUT_MONO -O2 -g -I. -I./$(MONO_DIR)/ $(SCHEME_SOURCES) `pkg-config --cflags --libs glib-2.0` -lpthread -lm
