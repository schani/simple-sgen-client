#include "mono/metadata/sgen-gc.h"
#include "mono/metadata/gc-internal-agnostic.h"

static GCVTable cons_vtable;

#define LIST_LENGTH	100000000

static void
check_list (GCObject *list, int length)
{
	GCObject *iter = list;
	for (int i = 0; i < length - 1; ++i) {
		g_assert (!iter->car);
		g_assert (iter->cdr);
		iter = iter->cdr;
	}
	g_assert (!iter->cdr);
}

static void
run (void)
{
	GCObject *cons = NULL;

	for (int i = 0; i < LIST_LENGTH; ++i) {
		GCObject *new = sgen_alloc_obj (&cons_vtable, sizeof (GCObject));
		g_assert (new->vtable == &cons_vtable);
		g_assert (!new->car && !new->cdr);
		mono_gc_wbarrier_generic_store (&new->cdr, cons);
		cons = new;
		//sgen_gc_collect (GENERATION_NURSERY);
		//check_list (cons, i + 1);
	}
	check_list (cons, LIST_LENGTH);
}

int
main (void)
{
	void *dummy;
	gsize cons_bitmap = 6;

	sgen_gc_init ();
	sgen_thread_register (&main_thread_info, &dummy);

	cons_vtable.descriptor = (mword)mono_gc_make_descr_for_object (&cons_bitmap, 2, sizeof (GCObject));

	run ();

	return 0;
}
