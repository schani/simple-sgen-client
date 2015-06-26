#ifdef SGEN_DEFINE_OBJECT_VTABLE

typedef mword GCVTable;

/* obj_t -- scheme object type
 *
 * obj_t is a pointer to a union, obj_u, which has members for
 * each scheme representation.
 *
 * The obj_u also has a "type" member.  Each representation
 * structure also has a "type" field first.  ANSI C guarantees
 * that these type fields correspond [section?].
 *
 * Objects are allocated by allocating one of the representation
 * structures and casting the pointer to it to type obj_t.  This
 * allows objects of different sizes to be represented by the
 * same type.
 *
 * To access an object, check its type by reading TYPE(obj), then
 * access the fields of the representation, e.g.
 *   if(TYPE(obj) == TYPE_PAIR) fiddle_with(CAR(obj));
 */

typedef union obj_u *obj_t;

typedef obj_t (*entry_t)(obj_t env, obj_t op_env, obj_t operator, obj_t rands);

typedef int type_t;
enum {
  TYPE_INVALID,
  TYPE_PAIR,
  TYPE_INTEGER,
  TYPE_SYMBOL,
  TYPE_SPECIAL,
  TYPE_OPERATOR,
  TYPE_STRING,
  TYPE_PORT,
  TYPE_PROMISE,
  TYPE_CHARACTER,
  TYPE_VECTOR,
  TYPE_TABLE,
  TYPE_BUCKETS,
  TYPE_ARRAY_FILL,
  TYPE_MAX
};

typedef struct pair_s {
  GCVTable vtable;			/* TYPE_PAIR */
  obj_t car, cdr;		/* first and second projections */
} pair_s;

typedef struct symbol_s {
  GCVTable vtable;			/* TYPE_SYMBOL */
  size_t length;		/* length of symbol string (excl. NUL) */
  char string[1];		/* symbol string, NUL terminated */
} symbol_s;

typedef struct integer_s {
  GCVTable vtable;			/* TYPE_INTEGER */
  long integer;			/* the integer */
} integer_s;

typedef struct special_s {
  GCVTable vtable;			/* TYPE_SPECIAL */
  char *name;			/* printed representation, NUL terminated */
} special_s;

typedef struct operator_s {
  GCVTable vtable;			/* TYPE_OPERATOR */
  obj_t arguments, body;	/* function arguments and code */
  obj_t env, op_env;		/* closure environments */
  char *name;			/* printed name, NUL terminated */
  entry_t entry;		/* entry point -- see eval() */
} operator_s;

typedef struct string_s {
  GCVTable vtable;			/* TYPE_STRING */
  size_t length;		/* number of chars in string */
  char string[1];		/* string, NUL terminated */
} string_s;

typedef struct port_s {
  GCVTable vtable;			/* TYPE_PORT */
  obj_t name;			/* name of stream */
  FILE *stream;
} port_s;

typedef struct character_s {
  GCVTable vtable;			/* TYPE_CHARACTER */
  char c;			/* the character */
} character_s;

typedef struct vector_s {
  GCVTable vtable;			/* TYPE_VECTOR */
  size_t length;		/* number of elements */
  obj_t vector[1];		/* vector elements */
} vector_s;

typedef unsigned long (*hash_t)(obj_t obj);
typedef int (*cmp_t)(obj_t obj1, obj_t obj2);

typedef struct table_s {
  GCVTable vtable;                  /* TYPE_TABLE */
  obj_t buckets;                /* hash buckets */
  hash_t hash;                  /* hash function */
  cmp_t cmp;                    /* comparison function */
} table_s;

typedef struct buckets_s {
  GCVTable vtable;                  /* TYPE_BUCKETS */
  size_t length;                /* number of buckets */
  size_t used;                  /* number of buckets in use */
  size_t deleted;               /* number of deleted buckets */
  struct bucket_s {
    obj_t key, value;
  } bucket[1];                  /* hash buckets */
} buckets_s;

typedef struct array_fill_s {
  GCVTable vtable;
  size_t size;
} array_fill_s;

typedef union obj_u {
  GCVTable vtable;			/* one of TYPE_* */
  pair_s pair;
  symbol_s symbol;
  integer_s integer;
  special_s special;
  operator_s operator;
  string_s string;
  port_s port;
  character_s character;
  vector_s vector;
  table_s table;
  buckets_s buckets;
  array_fill_s array_fill;
} GCObject;


/* structure macros */

#define VTABLE_TO_TYPE(vt)	((type_t)((vt) >> 3))
#define TYPE_TO_VTABLE(t)	((mword)(t) << 3)

static inline GCVTable
SGEN_LOAD_VTABLE_UNCHECKED (GCObject *obj)
{
	return obj->vtable;
}

extern mword descriptors_for_types [TYPE_MAX];

static inline mword
sgen_vtable_get_descriptor (GCVTable vtable)
{
	int type = VTABLE_TO_TYPE (vtable);
	g_assert (type > TYPE_INVALID && type < TYPE_ARRAY_FILL);
	return descriptors_for_types [type];
}

#include <setjmp.h>

typedef struct {
	jmp_buf registers;
	void *stack_bottom;
	void *stack_top;
} SgenClientThreadInfo;

#else

static size_t
symbol_size_for_length (size_t length)
{
	return offsetof(symbol_s, string) + length+1;
}

static size_t
string_size_for_length (size_t length)
{
	return offsetof(string_s, string) + length+1;
}

static size_t
vector_size_for_length (size_t length)
{
	return offsetof(vector_s, vector) + length * sizeof(obj_t);
}

static size_t
buckets_size_for_length (size_t length)
{
	return offsetof(buckets_s, bucket) + length * 2 * sizeof(obj_t);
}

static mword
sgen_client_slow_object_get_size (GCVTable vtable, GCObject* o)
{
  switch (VTABLE_TO_TYPE (vtable)) {
    case TYPE_PAIR:
    case TYPE_PROMISE:
      return sizeof (pair_s);
    case TYPE_INTEGER:
      return sizeof (integer_s);
    case TYPE_SYMBOL:
      return symbol_size_for_length (o->symbol.length);
    case TYPE_SPECIAL:
      return sizeof (special_s);
    case TYPE_OPERATOR:
      return sizeof (operator_s);
    case TYPE_STRING:
      return string_size_for_length (o->string.length);
    case TYPE_PORT:
      return sizeof (port_s);
    case TYPE_CHARACTER:
      return sizeof (character_s);
    case TYPE_VECTOR:
      return vector_size_for_length (o->vector.length);
    case TYPE_TABLE:
      return sizeof (table_s);
    case TYPE_BUCKETS:
      return buckets_size_for_length (o->buckets.length);
    case TYPE_ARRAY_FILL:
      return o->array_fill.size;
    default:
      g_assert_not_reached ();
  }
}

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

#define SGEN_CLIENT_OBJECT_HEADER_SIZE		(sizeof (GCVTable))
#define SGEN_CLIENT_MINIMUM_OBJECT_SIZE		(2 * sizeof (mword))

static inline size_t
sgen_client_array_element_size (GCVTable gc_vtable)
{
	switch (VTABLE_TO_TYPE (gc_vtable)) {
		case TYPE_VECTOR:
			return sizeof (obj_t);
		case TYPE_BUCKETS:
			return sizeof (struct bucket_s);
		default:
			g_assert_not_reached ();
	}
}

static inline char*
sgen_client_array_data_start (GCObject *obj)
{
	switch (VTABLE_TO_TYPE (obj->vtable)) {
		case TYPE_VECTOR:
			return (char*)obj->vector.vector;
		case TYPE_BUCKETS:
			return (char*)obj->buckets.bucket;
		default:
			g_assert_not_reached ();
	}
}

static inline size_t
sgen_client_array_length (GCObject *obj)
{
	switch (VTABLE_TO_TYPE (obj->vtable)) {
		case TYPE_VECTOR:
			return obj->vector.length;
		case TYPE_BUCKETS:
			return obj->buckets.length;
		default:
			g_assert_not_reached ();
	}
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
