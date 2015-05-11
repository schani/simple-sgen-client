/*
 * This file is included twice, first with `SGEN_DEFINE_OBJECT_VTABLE`
 * defined, the second time without.
 */

#ifdef SGEN_DEFINE_OBJECT_VTABLE

/*
 * The first word of an object is a `GCVTable`.  Usually that's a
 * pointer to a vtable structure.
 *
 * Note that a valid vtable cannot be NULL.
 */
typedef struct _VTable VTable;
typedef VTable* GCVTable;
struct _VTable {
	mword descriptor;
};

/*
 * All our objects have the same layout - they're cons pairs.
 */
typedef struct _GCObject GCObject;
struct _GCObject {
	GCVTable vtable;
	GCObject *car;
	GCObject *cdr;
};

/*
 * Actually, we do have a second kind of object, namely an array.
 * This type of object is not used by our application but by SGen
 * internally, to fill small holes in the nursery so that it is easily
 * walkable.
 */
typedef struct {
	GCVTable vtable;
	size_t size;
} GCArray;

/*
 * This is how we get an object's vtable.
 */
static inline GCVTable
SGEN_LOAD_VTABLE_UNCHECKED (GCObject *obj)
{
	return obj->vtable;
}

/*
 * This is how we get the GC descriptor from a vtable.
 *
 * If you don't need to store any additional information in the vtable
 * structure, you could typedef `GCVTable` to be `mword` and use the
 * descriptor as the vtable instead of getting it through an
 * indirection.
 */
static inline mword
sgen_vtable_get_descriptor (GCVTable vtable)
{
	return vtable->descriptor;
}

#include <setjmp.h>

/*
 * This is information we maintain for each thread.  The least we need
 * to be able to do is to scan its stack and registers.
 */
typedef struct {
	jmp_buf registers;
	void *stack_bottom;
	void *stack_top;
} SgenClientThreadInfo;

#else

/*
 * The object size slow path.  Our object size calculation is trivial,
 * so we only implement it once, here.
 */
static mword
sgen_client_slow_object_get_size (GCVTable vtable, GCObject* o)
{
	if ((vtable->descriptor & DESC_TYPE_MASK) == DESC_TYPE_VECTOR)
		return ((GCArray*)o)->size;
	return sizeof (GCObject);
}

/*
 * This is the object size fastpath.
 */
static mword
sgen_client_par_object_get_size (GCVTable vtable, GCObject* o)
{
	return sgen_client_slow_object_get_size (vtable, o);
}

#include <pthread.h>

typedef pthread_t MonoNativeThreadId;

extern pthread_key_t thread_info_key;
#ifdef HAVE_KW_THREAD
#define TLAB_ACCESS_INIT
#else
#define TLAB_ACCESS_INIT	SgenThreadInfo *__thread_info__ G_GNUC_UNUSED = mono_thread_info_current ()
#endif

typedef void* mono_native_thread_return_t;

#define mono_native_thread_id_get	pthread_self

static inline void
mono_native_thread_create (MonoNativeThreadId *thread, mono_native_thread_return_t (*func) (void*), void *user_data)
{
	int result = pthread_create (thread, NULL, func, user_data);
	g_assert (!result);
}

#define ENTER_CRITICAL_REGION
#define EXIT_CRITICAL_REGION

#define MONO_TRY_BLOCKING {
#define MONO_FINISH_TRY_BLOCKING }

#define mono_gc_printf	fprintf

extern SgenThreadInfo main_thread_info;

#define FOREACH_THREAD(thread)	thread = &main_thread_info;
#define END_FOREACH_THREAD

#include <sys/time.h>

#define SGEN_TV_DECLARE(tv)	struct timeval tv
#define SGEN_TV_GETTIME(tv)	gettimeofday (&(tv), NULL)

static inline gint64
SGEN_TV_ELAPSED (struct timeval a, struct timeval b)
{
	gint64 elapsed = (b.tv_sec - a.tv_sec) * (gint64)1000000;
	elapsed += b.tv_usec;
	elapsed -= a.tv_usec;
	return elapsed * 10;
}

#include <unistd.h>

static inline size_t
mono_pagesize (void)
{
	return getpagesize ();
}

static inline int
mono_process_current_pid (void)
{
	return getpid ();
}

#include <sys/mman.h>

enum {
	MONO_MMAP_NONE = 0,
	MONO_MMAP_READ = 1,
	MONO_MMAP_WRITE = 2,
	MONO_MMAP_PRIVATE = 4,
	MONO_MMAP_ANON = 8
};

void* mono_valloc     (void *addr, size_t length, int flags);
void* mono_valloc_aligned (size_t length, size_t alignment, int flags);
int   mono_vfree      (void *addr, size_t length);
int   mono_mprotect   (void *addr, size_t length, int flags);

enum {
	MONO_COUNTER_INT,
	MONO_COUNTER_WORD,     /* pointer-sized int */
	MONO_COUNTER_ULONG,    /* 64 bit uint */
	MONO_COUNTER_JIT       = 1 << 8,
	MONO_COUNTER_GC        = 1 << 9,
	MONO_COUNTER_BYTES     = 1 << 24, /* Quantity of bytes. RSS, active heap, etc */
	MONO_COUNTER_TIME      = 2 << 24,  /* Time interval in 100ns units. Minor pause, JIT compilation*/
	MONO_COUNTER_MONOTONIC = 1 << 28, /* This counter value always increase/decreases over time. Reported by --stat. */
	MONO_COUNTER_VARIABLE  = 1 << 30, /* This counter value can be anything on each sampling. Only interesting when sampling. */
};

void mono_counters_register (const char* descr, int type, void *addr);

enum {
	INTERNAL_MEM_MAX = INTERNAL_MEM_FIRST_CLIENT
};

#define SGEN_CLIENT_OBJECT_HEADER_SIZE		(sizeof (void*))
#define SGEN_CLIENT_MINIMUM_OBJECT_SIZE		(sizeof (GCObject))

static inline size_t
sgen_client_array_element_size (GCVTable gc_vtable)
{
	g_assert_not_reached ();
}

static inline char*
sgen_client_array_data_start (GCObject *obj)
{
	g_assert_not_reached ();
}

static inline size_t
sgen_client_array_length (GCObject *obj)
{
	g_assert_not_reached ();
}

static inline void
sgen_client_pre_copy_checks (char *destination, GCVTable gc_vtable, void *obj, mword objsize)
{
}

static inline void
sgen_client_update_copied_object (char *destination, GCVTable gc_vtable, void *obj, mword objsize)
{
}

#define sgen_client_wbarrier_generic_nostore_check(ptr)

#endif
