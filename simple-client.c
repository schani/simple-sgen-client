#include <glib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <setjmp.h>
#include <string.h>

#include "mono/metadata/sgen-gc.h"
#include "mono/metadata/sgen-client.h"
#include "mono/metadata/sgen-pinning.h"
#include "mono/metadata/gc-internal-agnostic.h"
#include "mono/utils/hazard-pointer.h"

SgenThreadInfo the_thread_info;
pthread_key_t thread_info_key;

void
sgen_client_init (void)
{
	pthread_key_create (&thread_info_key, NULL);
	pthread_setspecific (thread_info_key, &the_thread_info);
}

size_t
sgen_client_vtable_get_instance_size (GCVTable *vtable)
{
	return sgen_client_slow_object_get_size (vtable, NULL);
}

static GCVTable array_fill_vtable;

GCVTable*
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

void
sgen_client_object_register_finalizer_if_necessary (GCObject *obj)
{
	g_assert_not_reached ();
}

gboolean
sgen_client_object_has_critical_finalizer (GCObject *obj)
{
	g_assert_not_reached ();
}

void
sgen_client_object_queued_for_finalization (GCObject *obj)
{
	g_assert_not_reached ();
}

void
sgen_client_run_finalize (GCObject *obj)
{
	g_assert_not_reached ();
}

void
sgen_client_finalize_notify (void)
{
	g_assert_not_reached ();
}

gboolean
sgen_client_mark_ephemerons (ScanCopyContext ctx)
{
	return TRUE;
}

void
sgen_client_clear_unreachable_ephemerons (ScanCopyContext ctx)
{
}

gboolean
sgen_client_cardtable_scan_object (char *obj, mword block_obj_size, guint8 *cards, gboolean mod_union, SgenGrayQueue *queue)
{
	g_assert_not_reached ();
}

void
sgen_client_nursery_objects_pinned (void **definitely_pinned, int count)
{
}

void
sgen_client_collecting_minor (SgenPointerQueue *fin_ready_queue, SgenPointerQueue *critical_fin_queue)
{
}

void
sgen_client_collecting_major_1 (void)
{
}

void
sgen_client_pinned_los_object (char *obj)
{
	g_assert_not_reached ();
}

void
sgen_client_collecting_major_2 (void)
{
}

void
sgen_client_collecting_major_3 (SgenPointerQueue *fin_ready_queue, SgenPointerQueue *critical_fin_queue)
{
}

void
sgen_client_degraded_allocation (size_t size)
{
	g_assert_not_reached ();
}

void
sgen_client_total_allocated_heap_changed (size_t allocated_heap)
{
}

void
sgen_client_out_of_memory (size_t size)
{
	g_assert_not_reached ();
}

const
char* sgen_client_description_for_internal_mem_type (int type)
{
	g_assert_not_reached ();
}

const char*
sgen_client_object_safe_name (GCObject *obj)
{
	g_assert_not_reached ();
}

const char*
sgen_client_vtable_get_namespace (GCVTable *vtable)
{
	g_assert_not_reached ();
}

const char*
sgen_client_vtable_get_name (GCVTable *vtable)
{
	g_assert_not_reached ();
}

void
sgen_client_pre_collection_checks (void)
{
}

size_t
sgen_client_page_size (void)
{
	return getpagesize ();
}

void*
sgen_client_valloc (size_t size, gboolean activate)
{
	char *p = mmap (NULL, size, activate ? PROT_READ | PROT_WRITE : PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, 0);
	g_assert (p);
	return p;
}

static char*
aligned_address (char *mem, size_t size, size_t alignment)
{
	char *aligned = (char*)((size_t)(mem + (alignment - 1)) & ~(alignment - 1));
	g_assert (aligned >= mem && aligned + size <= mem + size + alignment && !((size_t)aligned & (alignment - 1)));
	return aligned;
}

void*
sgen_client_valloc_aligned (size_t size, size_t alignment, gboolean activate)
{
	char *p = sgen_client_valloc (size + alignment, activate);
	char *aligned = aligned_address (p, size, alignment);
	if (aligned != p)
		sgen_client_vfree (p, aligned - p);
	if (aligned + size != p + size + alignment)
		sgen_client_vfree (aligned + size, p + alignment - aligned);
	return aligned;
}

void
sgen_client_vfree (void *addr, size_t size)
{
	munmap (addr, size);
}

static void **stack_bottom;

void
sgen_client_thread_register (SgenThreadInfo* info, void *stack_bottom_fallback)
{
	stack_bottom = stack_bottom_fallback;
}

void
sgen_client_thread_unregister (SgenThreadInfo *p)
{
	g_assert_not_reached ();
}

void
sgen_client_thread_attach (SgenThreadInfo *info)
{
	g_assert_not_reached ();
}

void
sgen_client_thread_register_worker (void)
{
	g_assert_not_reached ();
}

void
sgen_client_scan_thread_data (void *start_nursery, void *end_nursery, gboolean precise, SgenGrayQueue *queue)
{
	void *dummy;
	jmp_buf registers;

	g_assert (stack_bottom && &dummy < stack_bottom);

	setjmp (registers);

	sgen_conservatively_pin_objects_from (&dummy, stack_bottom, start_nursery, end_nursery, PIN_TYPE_STACK);
	sgen_conservatively_pin_objects_from ((void**)registers, (void**)(((char*)registers) + sizeof (registers)), start_nursery, end_nursery, PIN_TYPE_STACK);
}

/* FIXME: remove */
gboolean
mono_gc_register_thread (void *baseptr)
{
	return TRUE;
}

void
sgen_client_stop_world (int generation)
{
}

void
sgen_client_restart_world (int generation, GGTimingInfo *timing)
{
}

gboolean
sgen_client_bridge_need_processing (void)
{
	return FALSE;
}

void
sgen_client_bridge_reset_data (void)
{
	g_assert_not_reached ();
}

void
sgen_client_bridge_processing_stw_step (void)
{
	g_assert_not_reached ();
}

void
sgen_client_bridge_wait_for_processing (void)
{
	g_assert_not_reached ();
}

void
sgen_client_bridge_processing_finish (int generation)
{
	g_assert_not_reached ();
}

gboolean
sgen_client_bridge_is_bridge_object (GCObject *obj)
{
	g_assert_not_reached ();
}

void
sgen_client_bridge_register_finalized_object (GCObject *object)
{
	g_assert_not_reached ();
}

void
sgen_client_log_timing (GGTimingInfo *info, mword last_major_num_sections, mword last_los_memory_usage)
{
}

gboolean
sgen_client_handle_gc_param (const char *opt)
{
	g_assert_not_reached ();
}

void
sgen_client_print_gc_params_usage (void)
{
	g_assert_not_reached ();
}

gboolean
sgen_client_handle_gc_debug (const char *opt)
{
	g_assert_not_reached ();
}

void
sgen_client_print_gc_debug_usage (void)
{
	g_assert_not_reached ();
}

void
sgen_client_protocol_collection_requested (int generation, size_t requested_size, gboolean force)
{
}

void
sgen_client_protocol_collection_begin (int minor_gc_count, int generation)
{
	printf ("collecting gen %d - %d\n", generation, minor_gc_count);
}

void
sgen_client_protocol_collection_end (int minor_gc_count, int generation, long long num_objects_scanned, long long num_unique_objects_scanned)
{
}

void
sgen_client_protocol_concurrent_start (void)
{
	g_assert_not_reached ();
}

void
sgen_client_protocol_concurrent_update (void)
{
	g_assert_not_reached ();
}

void
sgen_client_protocol_concurrent_finish (void)
{
	g_assert_not_reached ();
}

void
sgen_client_protocol_world_stopping (int generation)
{
}

void
sgen_client_protocol_world_stopped (int generation)
{
}

void
sgen_client_protocol_world_restarting (int generation)
{
}

void
sgen_client_protocol_world_restarted (int generation)
{
}

void
sgen_client_protocol_mark_start (int generation)
{
}

void
sgen_client_protocol_mark_end (int generation)
{
}

void
sgen_client_protocol_reclaim_start (int generation)
{
}

void
sgen_client_protocol_reclaim_end (int generation)
{
}

void
sgen_client_protocol_alloc (gpointer obj, gpointer vtable, size_t size, gboolean pinned)
{
}

void
sgen_client_protocol_alloc_degraded (gpointer obj, gpointer vtable, size_t size)
{
	g_assert_not_reached ();
}

void
sgen_client_protocol_pin (gpointer obj, gpointer vtable, size_t size)
{
}

void
sgen_client_protocol_cement (gpointer ptr, gpointer vtable, size_t size)
{
	g_assert_not_reached ();
}

void
sgen_client_protocol_copy (gpointer from, gpointer to, gpointer vtable, size_t size)
{
}

void
sgen_client_protocol_global_remset (gpointer ptr, gpointer value, gpointer value_vtable)
{
}

void
sgen_client_protocol_dislink_update (gpointer link, gpointer obj, gboolean track, gboolean staged)
{
	g_assert_not_reached ();
}

void
sgen_client_protocol_empty (gpointer start, size_t size)
{
}

void
sgen_client_counter_register_time (const char *name, guint64 *value, gboolean monotonic)
{
}

void
sgen_client_counter_register_uint64 (const char *name, guint64 *value)
{
}

void
sgen_client_counter_register_byte_count (const char *name, mword *value, gboolean monotonic)
{
}

static MonoThreadHazardPointers the_hazard_pointers;

gpointer
get_hazardous_pointer (gpointer volatile *pp, MonoThreadHazardPointers *hp, int hazard_index)
{
	gpointer p = *pp;
	if (!hp)
		return p;
	g_assert (hp == &the_hazard_pointers);
	g_assert (hazard_index >= 0 && hazard_index < HAZARD_POINTER_COUNT);
	hp->hazard_pointers [hazard_index] = p;
	return p;
}

MonoThreadHazardPointers*
mono_hazard_pointer_get (void)
{
	return &the_hazard_pointers;
}

void
mono_thread_hazardous_free_or_queue (gpointer p, MonoHazardousFreeFunc free_func,
		gboolean free_func_might_lock, gboolean lock_free_context)
{
	free_func (p);
}

void
mono_thread_hazardous_try_free_some (void)
{
}
