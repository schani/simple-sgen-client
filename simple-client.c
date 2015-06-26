#include <string.h>

#include "mono/sgen/sgen-gc.h"
#include "mono/sgen/sgen-client.h"
#include "mono/sgen/gc-internal-agnostic.h"

static VTable array_fill_vtable;

static GCVTable
sgen_client_get_array_fill_vtable (void)
{
	static gboolean inited = FALSE;

	if (!inited) {
		gsize bitmap = 0;
		array_fill_vtable.descriptor = (mword)mono_gc_make_descr_for_array (TRUE, &bitmap, 0, 1);
		inited = TRUE;
	}

	return &array_fill_vtable;
}

gboolean
sgen_client_array_fill_range (char *start, size_t size)
{
	GCArray *array;

	if (size < sizeof (GCArray)) {
		memset (start, 0, size);
		return FALSE;
	}

	array = (GCArray*)start;
	array->vtable = sgen_client_get_array_fill_vtable ();
	array->size = size;

	return TRUE;
}

void
sgen_client_zero_array_fill_header (void *p, size_t size)
{
	g_assert_not_reached ();
}

gboolean
sgen_client_object_is_array_fill (GCObject *o)
{
	return o->vtable == sgen_client_get_array_fill_vtable ();
}
