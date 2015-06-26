#include <string.h>

#include "mono/sgen/sgen-gc.h"
#include "mono/sgen/sgen-client.h"

gboolean
sgen_client_array_fill_range (char *start, size_t size)
{
	array_fill_s *fill;

	if (size < sizeof (array_fill_s)) {
		memset (start, 0, size);
		return FALSE;
	}

	fill = (array_fill_s*)start;
	fill->vtable = TYPE_TO_VTABLE (TYPE_ARRAY_FILL);
	fill->size = size;

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
	return VTABLE_TO_TYPE (o->vtable) == TYPE_ARRAY_FILL;
}
